#ifndef UTIL_H
#define UTIL_H

#include "../../external/json.h/json.h"

#define JSON_BUFSIZ 1000000

struct json_value_s *util_json_from_file(const char *path);
int util_strcat(char *dst, size_t size, const char *fmt, ...);

#endif
