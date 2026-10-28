# Embedded C Scientific Calculator & GUI

A lightweight, register-constrained scientific expression evaluator written in C11 with an optional Python Tkinter desktop interface.

Developed as a mini-project for an **Embedded Automotive Systems** minor course to explore mathematical parsing under automotive/microcontroller safety constraints.

---

## Motivation & Architecture

In automotive ECUs and embedded microcontrollers (like ARM Cortex-M or PIC/AVR), dynamic heap memory allocation (`malloc`/`free`) is forbidden or strongly discouraged due to fragmentation risks and non-deterministic execution times. Memory is tightly constrained, and code must adhere to strict safety standards such as MISRA C.

The core question behind this project was:  
**Can we parse and evaluate complex scientific expressions using only 3 double variables, zero heap allocations, and deterministic memory bounds?**

### The 3-Register Evaluation Model

Rather than allocating an Abstract Syntax Tree (AST) or token arrays in memory, the engine streams expressions on-the-fly through three dedicated registers:

- `reg[0] (Accumulator)`: Holds running expression-level additions and subtractions (`+`, `-`).
- `reg[1] (Term Register)`: Holds running term-level multiplications and divisions (`*`, `/`).
- `reg[2] (Temp / Operand)`: Holds the current factor, power (`^`), numeric literal, constant, or `#` function result.

### Embedded & MISRA C Safety Practices

The core calculation library (`calculator.c/.h`, `parser.c/.h`, `command.c/.h`) follows key embedded safety principles:

1. **Zero Dynamic Allocation**: No `malloc`, `calloc`, or `free`. All structures are statically sized or allocated on the automatic stack.
2. **Predictable Stack Footprint**: Recursion depth is tracked dynamically and strictly capped (`MAX_RECURSION_DEPTH = 32`) to prevent stack overflow on microcontroller stacks.
3. **No `<errno.h>` or `strtod()`**: Implements a self-contained, deterministic floating-point parser that directly reads integer, fractional, and scientific exponent notations.
4. **Decoupled from `<stdio.h>`**: The calculation engine has zero dependencies on standard I/O. Console I/O is kept strictly inside `main.c`.
5. **Fixed-Width Sized Types**: Uses `<stdint.h>` and `<stdbool.h>` types (`int32_t`, `uint32_t`, `size_t`, `bool`) for portable, unambiguous bit-widths.
6. **Structured Control Flow**: Every function in the core engine follows the single-point-of-exit rule (one `return` at the end).

---

## Project Structure

```text
├── Makefile               # GNU Make build configuration
├── CMakeLists.txt         # Alternative CMake configuration
├── main.c                 # Standalone CLI REPL and GUI pipe bridge
├── calculator.c / .h      # Public API facade
├── parser.c / .h          # 3-register expression parser and evaluator
├── command.c / .h         # Extensible '#' math function dispatch table
└── gui/
    ├── calculator_gui.py  # Python Tkinter desktop GUI
    └── README.md          # GUI notes
```

---

## Supported Operations

### Grammar & Precedence
- **Operators**: `+`, `-`, `*`, `/`, `^` (right-associative)
- **Grouping**: Parentheses `( expression )`
- **Constants**: `pi`, `e`, `ans` (reusable result of previous calculation)
- **Scientific Notation**: e.g., `1e-3`, `2.5E4`

### Built-in Commands (`#` Function Dispatcher)
Commands use the `#code(x)` syntax (20 functions supported):
- **Trigonometry**: `#1(x)` cos, `#2(x)` tan, `#4(x)` sin
- **Inverse Trig**: `#9(x)` asin, `#10(x)` acos, `#11(x)` atan
- **Hyperbolic**: `#12(x)` sinh, `#13(x)` cosh, `#14(x)` tanh
- **Logarithms & Exp**: `#5(x)` log10 ($x > 0$), `#6(x)` ln ($x > 0$), `#7(x)` exp ($e^x$)
- **Roots & Powers**: `#3(x)` sqrt ($x \ge 0$), `#18(x)` cbrt ($\sqrt[3]{x}$)
- **Rounding & Misc**: `#8(x)` abs, `#15(x)` floor, `#16(x)` ceil, `#17(x)` round
- **Angle Conversion**: `#19(x)` deg (rad to deg), `#20(x)` rad (deg to rad)

---

## Building and Running

### 1. Build with GNU Make (Recommended)

```bash
make
```

To clean build artifacts:
```bash
make clean
```

### 2. Build with CMake (Alternative)

```bash
cmake -S . -B build
cmake --build build
```

### 3. Run the CLI Interactive Mode

```bash
./scientific_calculator.exe
```

Example session:
```text
> 1+4/10*20
= 9

> 5*(2+3)
= 25

> 2^3^2
= 512

> 10*#1(0)
= 10
```

### 4. Run the Desktop GUI

The Python GUI acts as a visual front-end testbench that communicates with the compiled C binary:

```bash
python gui/calculator_gui.py
```
