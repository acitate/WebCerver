#ifndef __RESOURCE_RESOLVER_H
#define __RESOURCE_RESOLVER_H

typedef enum {
    PATH_OK = 0,
    PATH_ERR_MALFORMED_ENCODING,
    PATH_ERR_NULL_BYTE,       
    PATH_ERR_CONTROL_CHAR,
    PATH_ERR_TOO_LONG,         
    PATH_ERR_HIDDEN_PATH,         
    PATH_ERR_NOT_FOUND,           
    PATH_ERR_FORBIDDEN,
    PATH_ERR_IS_DIRECTORY,           
    PATH_ERR_UNEXPECTED,                  
} PathStatus;

PathStatus url_decode_path(const char *encoded, size_t encoded_len, char *out, size_t out_size);
PathStatus canonicalize_webroot(const char *webroot_input, char *out_canonical, size_t out_size);
PathStatus resolve_and_sanitize_path(const char *decoded_path, const char *canonical_webroot, char *out_resolved, size_t out_size);
PathStatus resolve_resource(const char *raw_path, size_t raw_path_len, const char *canonical_webroot, char **out_buf, size_t *out_len);

#endif