#include "utils.h"

/* String manipulation utilities */

char* string_duplicate(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* dup = (char*)safe_malloc(len + 1);
    strcpy(dup, str);
    return dup;
}

char* string_concat(const char* str1, const char* str2) {
    if (!str1) return string_duplicate(str2);
    if (!str2) return string_duplicate(str1);

    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char* result = (char*)safe_malloc(len1 + len2 + 1);

    strcpy(result, str1);
    strcat(result, str2);
    return result;
}

int string_equals(const char* str1, const char* str2) {
    if (!str1 || !str2) return 0;
    return strcmp(str1, str2) == 0;
}

int string_starts_with(const char* str, const char* prefix) {
    if (!str || !prefix) return 0;
    size_t len = strlen(prefix);
    return strncmp(str, prefix, len) == 0;
}

/* Memory management utilities */

void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        log_message(LOG_ERROR, "Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void* safe_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr && size > 0) {
        log_message(LOG_ERROR, "Memory reallocation failed");
        exit(EXIT_FAILURE);
    }
    return new_ptr;
}

/* File utilities */

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        log_message(LOG_ERROR, "Cannot open file: %s", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* content = (char*)safe_malloc(length + 1);
    size_t read_size = fread(content, 1, length, file);
    content[read_size] = '\0';

    fclose(file);
    return content;
}

int write_file(const char* filename, const char* content) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        log_message(LOG_ERROR, "Cannot write to file: %s", filename);
        return 0;
    }

    fprintf(file, "%s", content);
    fclose(file);
    return 1;
}

/* Logging and error reporting */

void log_message(LogLevel level, const char* format, ...) {
    const char* level_str[] = {"INFO", "WARNING", "ERROR"};
    fprintf(stderr, "[%s] ", level_str[level]);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
}

void report_error(const char* filename, int line, int column, const char* message) {
    fprintf(stderr, "[ERROR] %s:%d:%d: %s\n", filename, line, column, message);
}

/* Character classification helpers */

int is_identifier_start(char c) {
    return isalpha(c) || c == '_';
}

int is_identifier_char(char c) {
    return isalnum(c) || c == '_';
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}
