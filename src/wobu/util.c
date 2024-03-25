#include "../../external/json.h/json.h"
#include "SDL.h" // IWYU pragma: keep //clangd

#include "util.h"

struct json_value_s *util_json_from_file(const char *path)
{
    char buffer[JSON_BUFSIZ] = {0};
    SDL_RWops *file = SDL_RWFromFile(path, "r");

    if (file == NULL) {
        SDL_Log("Error opening json file: %s", path);
        return NULL;
    }
    for (size_t i = 0; i < JSON_BUFSIZ; i++) {
        if (SDL_RWread(file, &buffer[i], sizeof(char), 1) <= 0) {
            buffer[i] = 0;
            break;
        }
    }
    SDL_RWclose(file);

    size_t len = SDL_strlen(buffer);
    SDL_Log("JSON LOADED: %s [%zu bytes]", path, len);
    return json_parse(buffer, len);
}

int util_strcat(char *dst, size_t size, const char *fmt, ...)
{
    char buffer[512] = {0};
    va_list args;
    va_start(args, fmt);
    size_t len = SDL_vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len >= sizeof(buffer)) {
        SDL_Log("util_strcat error: buffer overflow");
        return -1;
    }

    len = SDL_strlcat(dst, buffer, size);
    if (len >= size) {
        SDL_Log("util_strcat error: string destination overflow");
        return -1;
    }
    return 0;
}
