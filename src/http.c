#include <stdio.h>
#include <stdlib.h>
#include "sds.h"
#include "http.h"
#include <string.h>
#include <stddef.h>


size_t find_first(const char *str, size_t str_len, const char *sub_str, size_t sub_str_len)
{
    // Finds first occurrence of `sub_str` in `str` and returns its index.
    if (sub_str_len == 0) return 0;
    if (sub_str_len > str_len) return SIZE_MAX;

    for (size_t i = 0; i <= str_len - sub_str_len; i++) {
        size_t j = 0;
        while (j < sub_str_len && str[i + j] == sub_str[j]) j++;
        if (j == sub_str_len) return i;
    }
    return SIZE_MAX;
}


void split_request(sds raw, size_t raw_len, sds *req_line, sds *headers, sds *body)
{
    // Split raw request string to request line, Headers and body. 
    size_t idx = find_first(raw, raw_len, "\r\n\r\n", 4); // First occurrence of `\r\n\r\n` separating headers from body
    size_t line_end = find_first(raw, raw_len, "\r\n", 2); // First occurrence of `\r\n` signifying the end of request line
    
    *body = sdsnew(raw);
    sdsrange(*body, idx + 4, raw_len);

    *req_line = sdsnew(raw);
    sdsrange(*req_line, 0, line_end);

    *headers = sdsnew(raw);
    sdsrange(*headers, line_end + 2, idx);
}


