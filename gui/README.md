# Python GUI for the Embedded C Scientific Calculator

This GUI uses Python Tkinter for the desktop interface and delegates expression evaluation to the compiled C calculator engine via standard process pipes.

## Features

- **Non-Modal Commands Window**: Keep the command list open side-by-side while freely using the calculator, with one-click **Insert** buttons for all functions.
- **`# ▾` Function Dropdown**: Click the `# ▾` button to select from all built-in mathematical functions (`#1 cos` to `#8 abs`), which automatically inserts the command with cursor placed inside the parentheses.
- **Both `⌫` (Backspace) and `DEL` (Forward Delete)**: Easy single-character and selection editing.
- **Dark Theme**: Modern styling matching scientific and automotive tooling.

## Requirements

- Python 3.9+
- Tkinter (included with standard Python on Windows)
- Compiled `scientific_calculator.exe` (run `make` in project root)

## Quick Start

1. Compile the C backend from the repository root:
   ```bash
   make
   ```

2. Run the desktop GUI:
   ```bash
   python gui/calculator_gui.py
   ```
