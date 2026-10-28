#include "calculator.h"
#include "parser.h"

#include <math.h>

static double current_ans = 0.0;

void calculator_set_ans(double value)
{
    if (isfinite(value) != 0) {
        current_ans = value;
    }
}

double calculator_get_ans(void)
{
    return current_ans;
}

CalcResult calculator_evaluate(const char *expression)
{
    CalcResult result = parser_parse(expression);
    if (result.status == CALC_OK) {
        calculator_set_ans(result.value);
    }
    return result;
}

const char *calculator_status_string(CalcStatus status)
{
    const char *str;

    switch (status) {
        case CALC_OK:
            str = "OK";
            break;
        case CALC_EMPTY_INPUT:
            str = "Empty input";
            break;
        case CALC_SYNTAX_ERROR:
            str = "Syntax error";
            break;
        case CALC_UNKNOWN_COMMAND:
            str = "Unknown command";
            break;
        case CALC_MISSING_ARGUMENT:
            str = "Missing argument";
            break;
        case CALC_DIVISION_BY_ZERO:
            str = "Division by zero";
            break;
        case CALC_DOMAIN_ERROR:
            str = "Domain error";
            break;
        case CALC_OVERFLOW:
            str = "Overflow";
            break;
        case CALC_DEPTH_EXCEEDED:
            str = "Recursion depth exceeded";
            break;
        case CALC_TRAILING_INPUT:
            str = "Trailing input";
            break;
        default:
            str = "Unknown error";
            break;
    }

    return str;
}
