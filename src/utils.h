#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* Version information */
#define C2EN_VERSION_MAJOR 1
#define C2EN_VERSION_MINOR 0
#define C2EN_VERSION_PATCH 0
#define C2EN_VERSION_STRING "1.0.0"

/* Error severity levels */
typedef enum {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
} LogLevel;

/* String manipulation utilities */
char* string_duplicate(const char* str);
char* string_concat(const char* str1, const char* str2);
int string_equals(const char* str1, const char* str2);
int string_starts_with(const char* str, const char* prefix);

/* Memory management utilities */
void* safe_malloc(size_t size);
void* safe_realloc(void* ptr, size_t size);

/* File utilities */
char* read_file(const char* filename);
int write_file(const char* filename, const char* content);

/* Logging and error reporting */
void log_message(LogLevel level, const char* format, ...);
void report_error(const char* filename, int line, int column, const char* message);

/* Character classification helpers */
int is_identifier_start(char c);
int is_identifier_char(char c);
int is_digit(char c);

#endif /* UTILS_H */
