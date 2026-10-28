#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    CALC_OK = 0,
    CALC_EMPTY_INPUT,
    CALC_SYNTAX_ERROR,
    CALC_UNKNOWN_COMMAND,
    CALC_MISSING_ARGUMENT,
    CALC_DIVISION_BY_ZERO,
    CALC_DOMAIN_ERROR,
    CALC_OVERFLOW,
    CALC_DEPTH_EXCEEDED,
    CALC_TRAILING_INPUT
} CalcStatus;

#define CALC_MSG_CAPACITY 160U

typedef struct {
    CalcStatus status;
    double value;
    size_t position;
    char message[CALC_MSG_CAPACITY];
} CalcResult;

CalcResult calculator_evaluate(const char *expression);
const char *calculator_status_string(CalcStatus status);

/* Getter and setter for the 'ans' (last result) register */
void calculator_set_ans(double value);
double calculator_get_ans(void);

#endif /* CALCULATOR_H */
