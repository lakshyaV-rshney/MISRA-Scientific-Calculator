#ifndef PARSER_H
#define PARSER_H

#include "calculator.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Maximum recursion depth allowed during parsing.
 * Keeps stack usage bounded and predictable for microcontroller / embedded targets
 * (in line with MISRA C guidelines on preventing stack overflow).
 */
#define MAX_RECURSION_DEPTH 32U

/*
 * Parser State
 * ------------
 * Evaluates expressions using a 3-register memory model:
 *   reg[0]: Expression accumulator (for + and -)
 *   reg[1]: Term accumulator (for * and /)
 *   reg[2]: Factor / temporary register (for numbers, ^, unary, and function calls)
 *
 * No dynamic memory allocation (malloc/free) is used.
 */
typedef struct {
    const char *input;
    const char *cursor;
    CalcResult result;
    uint32_t depth;
    double reg[3]; /* 3 hardware-like registers for streaming evaluation */
} Parser;

CalcResult parser_parse(const char *expression);

#endif /* PARSER_H */
