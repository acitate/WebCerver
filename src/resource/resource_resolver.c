#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "resource_resolver.h"
#include "filesystem.h"

#define PATH_MAX 4096
#define _XOPEN_SOURCE 700
#define RR_MAX_RESOLVED_PATH 4096
#define RR_MAX_DECODED_PATH 2048


static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}


PathStatus url_decode_path(const char *encoded, size_t encoded_len,
                            char *out, size_t out_size) {
    if (out_size == 0) return PATH_ERR_TOO_LONG;

    size_t o = 0;
    for (size_t i = 0; i < encoded_len; i++) {
        unsigned char c = (unsigned char)encoded[i];
        unsigned char decoded;

        if (c == '%') {
            if (i + 2 >= encoded_len) return PATH_ERR_MALFORMED_ENCODING;
            int hi = hex_nibble(encoded[i + 1]);
            int lo = hex_nibble(encoded[i + 2]);
            if (hi < 0 || lo < 0) return PATH_ERR_MALFORMED_ENCODING;
            decoded = (unsigned char)((hi << 4) | lo);
            i += 2;
        } else {
            decoded = c;
        }

        if (decoded == '\0') return PATH_ERR_NULL_BYTE;

        if (decoded < 0x20 || decoded == 0x7f) return PATH_ERR_CONTROL_CHAR;

        if (o + 1 >= out_size) return PATH_ERR_TOO_LONG; /* leave room for NUL */
        out[o++] = (char)decoded;
    }

    out[o] = '\0';
    return PATH_OK;
}


PathStatus canonicalize_webroot(const char *webroot_input,
                                 char *out_canonical, size_t out_size) {
    char resolved[PATH_MAX];
    if (realpath(webroot_input, resolved) == NULL) {
        return (errno == ENOENT) ? PATH_ERR_NOT_FOUND : PATH_ERR_UNEXPECTED;
    }
    if (strlen(resolved) + 1 > out_size) return PATH_ERR_TOO_LONG;
    strcpy(out_canonical, resolved);
    return PATH_OK;
}


static int has_hidden_component(const char *path)
{
    const char *p = path;
    while (*p == '/') p++;
    while (*p) {
        if (*p == '.') return 1;
        while (*p && *p != '/') p++;
        while (*p == '/') p++;
    }
    return 0;
}

PathStatus resolve_and_sanitize_path(const char *decoded_path, const char *canonical_webroot, char *out_resolved, size_t out_size)
{
    size_t path_len = strlen(decoded_path);
    if (path_len == 0 || path_len >= RR_MAX_DECODED_PATH) {
        return PATH_ERR_TOO_LONG;
    }

    if (has_hidden_component(decoded_path)) {
        return PATH_ERR_HIDDEN_PATH;
    }

    /* Always join against the fixed webroot -- never treat the
     * request path as a standalone filesystem path. Even an
     * absolute-looking "/etc/passwd" becomes webroot + "/etc/passwd". */
    char candidate[PATH_MAX];
    int n = snprintf(candidate, sizeof(candidate), "%s%s",
                      canonical_webroot, decoded_path);
    if (n < 0 || (size_t)n >= sizeof(candidate)) return PATH_ERR_TOO_LONG;

    char resolved[PATH_MAX];
    if (realpath(candidate, resolved) == NULL) {
        switch (errno) {
            case ENOENT: return PATH_ERR_NOT_FOUND;
            case EACCES: return PATH_ERR_FORBIDDEN;
            default:     return PATH_ERR_UNEXPECTED;
        }
    }

    /* The core containment check: resolved must be the webroot itself
     * or a true subpath of it -- not merely share a string prefix
     * (webroot "/var/www" must not match "/var/wwwevil/secret"). This
     * is also what catches a symlink inside the webroot pointing
     * somewhere else, since realpath() above already followed it. */
    size_t webroot_len = strlen(canonical_webroot);
    if (strncmp(resolved, canonical_webroot, webroot_len) != 0 ||
        (resolved[webroot_len] != '/' && resolved[webroot_len] != '\0')) {
        return PATH_ERR_FORBIDDEN;
    }

    if (strlen(resolved) + 1 > out_size) return PATH_ERR_TOO_LONG;
    strcpy(out_resolved, resolved);
    return PATH_OK;
}


static PathStatus map_fs_status(FsStatus fs)
{
    switch (fs) {
        case FS_OK:                    return PATH_OK;
        case FS_ERR_NOT_FOUND:         return PATH_ERR_NOT_FOUND;
        case FS_ERR_IS_DIRECTORY:      return PATH_ERR_IS_DIRECTORY;
        case FS_ERR_PERMISSION_DENIED: return PATH_ERR_FORBIDDEN;
        default:                       return PATH_ERR_UNEXPECTED;
    }
}


PathStatus resolve_resource(const char *raw_path, size_t raw_path_len, const char *canonical_webroot, char **out_buf, size_t *out_len)
{
    char decoded[RR_MAX_DECODED_PATH];
    PathStatus s = url_decode_path(raw_path, raw_path_len, decoded, sizeof(decoded));
    if (s != PATH_OK) return s;

    char resolved[RR_MAX_RESOLVED_PATH];
    s = resolve_and_sanitize_path(decoded, canonical_webroot, resolved, sizeof(resolved));
    if (s != PATH_OK) return s;

    /* Cache lookup slots in here once cache.c exists:
     *
     *   if (cache_get(resolved, out_buf, out_len) == CACHE_HIT) return PATH_OK;
     *
     * resolve_resource() would then be the one place that decides
     * cache-vs-disk, per the earlier design -- filesystem.c and
     * cache.c stay ignorant of each other. */

    FsStatus fs = filesystem_read_file(resolved, out_buf, out_len);
    s = map_fs_status(fs);

    /* Cache populate slots in here on a successful disk read:
     *
     *   if (s == PATH_OK) cache_put(resolved, *out_buf, *out_len);
     */

    return s;
}