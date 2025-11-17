#ifndef FORMATTER_H
#define FORMATTER_H

#include "utils.h"

/* Formatter functions */
char* format_english_output(const char* raw_output);

/* British English spelling conversions */
char* apply_british_spelling(const char* text);

#endif /* FORMATTER_H */
