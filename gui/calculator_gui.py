import ctypes
import os
import platform
import subprocess
import sys
import tkinter as tk
from tkinter import messagebox, ttk
from pathlib import Path


class CalculatorBackend:
    """Bridge the Python GUI to the existing C calculator executable using a persistent REPL process."""

    def __init__(self):
        self.project_dir = Path(__file__).resolve().parent.parent
        self.executable = self._find_executable()
        self.process = None
        self._start_process()

    def _find_executable(self):
        candidates = [
            self.project_dir / "scientific_calculator.exe",
            self.project_dir / "build" / "scientific_calculator.exe",
            self.project_dir / "build" / "Release" / "scientific_calculator.exe",
            self.project_dir / "scientific_calculator",
            self.project_dir / "build" / "scientific_calculator",
        ]

        for candidate in candidates:
            if candidate.exists():
                return candidate

        return None

    def _start_process(self):
        if self.executable is None or not self.executable.exists():
            return

        try:
            self.process = subprocess.Popen(
                [str(self.executable)],
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                bufsize=1,
            )
            # Read header until first interactive prompt
            self._read_until_prompt()
        except Exception:
            self.process = None

    def _read_until_prompt(self):
        if self.process is None:
            return ""

        output = []
        try:
            while True:
                char = self.process.stdout.read(1)
                if not char:
                    break
                output.append(char)
                if len(output) >= 2 and output[-2] == ">" and output[-1] == " ":
                    break
        except Exception:
            pass
        return "".join(output)

    def evaluate(self, expression):
        if not expression.strip():
            return "", "Expression is empty."

        if self.executable is None:
            return "", (
                "C calculator executable was not found.\n\n"
                "Build the C project first (e.g. run 'make'), then launch the GUI."
            )

        if self.process is None or self.process.poll() is not None:
            self._start_process()

        if self.process is None:
            return "", "Failed to start C calculator backend."

        try:
            # Send expression to the C REPL
            self.process.stdin.write(f"{expression}\n")
            self.process.stdin.flush()

            # Read response until prompt
            response_raw = self._read_until_prompt()
            lines = [line.strip() for line in response_raw.splitlines() if line.strip()]

            ans_line = ""
            for line in lines:
                if line.startswith("= ") or line.startswith("Error ["):
                    ans_line = line
                    break

            if ans_line.startswith("= "):
                return ans_line[2:], ""
            elif ans_line.startswith("Error ["):
                return "", ans_line
            elif lines:
                return lines[0], ""
            else:
                return "", "No response from C calculator."
        except Exception as exc:
            self._start_process()
            return "", f"Backend process error: {exc}"

    def close(self):
        if self.process and self.process.poll() is None:
            try:
                self.process.stdin.write("exit\n")
                self.process.stdin.flush()
                self.process.wait(timeout=1)
            except Exception:
                self.process.kill()


class ScientificCalculatorGUI(tk.Tk):
    """Desktop GUI for the modular C scientific calculator."""

    BG = "#111318"
    PANEL = "#191c22"
    DISPLAY = "#0c0e12"
    TEXT = "#f2f4f8"
    MUTED = "#9ca3af"
    ACCENT = "#5b8def"
    BUTTON = "#252a33"
    BUTTON_HOVER = "#303744"
    OPERATOR = "#343b49"
    DANGER = "#8f3f46"

    # Expanded 20 built-in mathematical commands matching C engine
    COMMANDS = [
        ("1",  "cos",   "Cosine (radians)", "Trig"),
        ("2",  "tan",   "Tangent (radians)", "Trig"),
        ("3",  "sqrt",  "Square root (x >= 0)", "Roots & Powers"),
        ("4",  "sin",   "Sine (radians)", "Trig"),
        ("5",  "log",   "Base-10 logarithm (x > 0)", "Log & Exp"),
        ("6",  "ln",    "Natural logarithm (x > 0)", "Log & Exp"),
        ("7",  "exp",   "e raised to x", "Log & Exp"),
        ("8",  "abs",   "Absolute value |x|", "Rounding & Misc"),
        ("9",  "asin",  "Arcsine (-1 <= x <= 1)", "Inverse Trig"),
        ("10", "acos",  "Arccosine (-1 <= x <= 1)", "Inverse Trig"),
        ("11", "atan",  "Arctangent", "Inverse Trig"),
        ("12", "sinh",  "Hyperbolic sine", "Hyperbolic"),
        ("13", "cosh",  "Hyperbolic cosine", "Hyperbolic"),
        ("14", "tanh",  "Hyperbolic tangent", "Hyperbolic"),
        ("15", "floor", "Floor: greatest integer <= x", "Rounding & Misc"),
        ("16", "ceil",  "Ceil: least integer >= x", "Rounding & Misc"),
        ("17", "round", "Round to nearest integer", "Rounding & Misc"),
        ("18", "cbrt",  "Cube root", "Roots & Powers"),
        ("19", "deg",   "Convert radians to degrees", "Angle Conversion"),
        ("20", "rad",   "Convert degrees to radians", "Angle Conversion"),
    ]

    def __init__(self):
        super().__init__()

        self.title("Scientific Calculator (Embedded C Engine)")
        self.geometry("480x730")
        self.minsize(420, 650)
        self.configure(bg=self.BG)

        self.backend = CalculatorBackend()
        self.expression = tk.StringVar()
        self.status = tk.StringVar(value="Ready")
        self._last_result = ""
        self._commands_window = None
        self._cmd_button = None

        self._configure_style()
        self._build_ui()
        self._create_command_menu()
        self._bind_keys()

        self.protocol("WM_DELETE_WINDOW", self._on_close)

    def _configure_style(self):
        style = ttk.Style(self)

        try:
            style.theme_use("clam")
        except tk.TclError:
            pass

        style.configure(
            "TButton",
            font=("Segoe UI", 11),
            padding=10,
            foreground=self.TEXT,
            background=self.BUTTON,
            borderwidth=0,
        )

        style.map(
            "TButton",
            background=[("active", self.BUTTON_HOVER)],
        )

    def _build_ui(self):
        header = tk.Frame(self, bg=self.BG)
        header.pack(fill="x", padx=20, pady=(18, 8))

        tk.Label(
            header,
            text="Scientific Calculator",
            font=("Segoe UI", 20, "bold"),
            bg=self.BG,
            fg=self.TEXT,
        ).pack(side="left")

        tk.Button(
            header,
            text="Commands (20) 📋",
            command=self._show_commands,
            font=("Segoe UI", 10, "bold"),
            bg=self.PANEL,
            fg=self.TEXT,
            activebackground=self.BUTTON_HOVER,
            activeforeground=self.TEXT,
            relief="flat",
            padx=14,
            pady=7,
            cursor="hand2",
        ).pack(side="right")

        display_frame = tk.Frame(
            self,
            bg=self.DISPLAY,
            highlightthickness=1,
            highlightbackground="#292e38",
        )
        display_frame.pack(fill="x", padx=20, pady=(8, 10))

        self.display = tk.Entry(
            display_frame,
            textvariable=self.expression,
            font=("Cascadia Mono", 24),
            bg=self.DISPLAY,
            fg=self.TEXT,
            insertbackground=self.TEXT,
            selectbackground=self.ACCENT,
            selectforeground=self.TEXT,
            relief="flat",
            justify="right",
            bd=0,
        )
        self.display.pack(fill="x", padx=16, pady=(18, 8), ipady=8)
        self.display.focus_set()

        tk.Label(
            display_frame,
            textvariable=self.status,
            font=("Segoe UI", 9),
            bg=self.DISPLAY,
            fg=self.MUTED,
            anchor="e",
        ).pack(fill="x", padx=18, pady=(0, 10))

        buttons = tk.Frame(self, bg=self.BG)
        buttons.pack(fill="both", expand=True, padx=20, pady=8)

        layout = [
            ("(", "(", "normal"),
            (")", ")", "normal"),
            ("π", "pi", "normal"),
            ("e", "e", "normal"),
            ("⌫", "BACKSPACE", "danger"),

            ("7", "7", "normal"),
            ("8", "8", "normal"),
            ("9", "9", "normal"),
            ("÷", "/", "operator"),
            ("C", "CLEAR", "danger"),

            ("4", "4", "normal"),
            ("5", "5", "normal"),
            ("6", "6", "normal"),
            ("×", "*", "operator"),
            ("^", "^", "operator"),

            ("1", "1", "normal"),
            ("2", "2", "normal"),
            ("3", "3", "normal"),
            ("−", "-", "operator"),
            ("+", "+", "operator"),

            (".", ".", "normal"),
            ("0", "0", "normal"),
            ("# ▾", "COMMAND_MENU", "command"),
            ("Ans", "ANS", "command"),
            ("=", "EQUALS", "equals"),
        ]

        for index, (label, action, kind) in enumerate(layout):
            row = index // 5
            column = index % 5

            bg = self.BUTTON
            if kind == "operator":
                bg = self.OPERATOR
            elif kind == "danger":
                bg = self.DANGER
            elif kind == "command":
                bg = "#3b4260"
            elif kind == "equals":
                bg = self.ACCENT

            button = tk.Button(
                buttons,
                text=label,
                command=lambda a=action, l=label: self._handle_action(a),
                font=("Segoe UI", 12, "bold"),
                bg=bg,
                fg=self.TEXT,
                activebackground=self.BUTTON_HOVER,
                activeforeground=self.TEXT,
                relief="flat",
                bd=0,
                cursor="hand2",
            )

            if action == "COMMAND_MENU":
                self._cmd_button = button

            button.grid(
                row=row,
                column=column,
                sticky="nsew",
                padx=4,
                pady=4,
                ipadx=3,
                ipady=10,
            )

        for row in range(5):
            buttons.rowconfigure(row, weight=1)

        for column in range(5):
            buttons.columnconfigure(column, weight=1)

        footer = tk.Label(
            self,
            text="Supports 20 functions via #code(x) and native 'ans' in C",
            font=("Segoe UI", 9),
            bg=self.BG,
            fg=self.MUTED,
        )
        footer.pack(pady=(4, 12))

    def _create_command_menu(self):
        """Dropdown menu categorized by function groups."""
        self.cmd_menu = tk.Menu(
            self,
            tearoff=0,
            bg=self.PANEL,
            fg=self.TEXT,
            activebackground=self.ACCENT,
            activeforeground=self.TEXT,
            font=("Segoe UI", 10),
            relief="flat",
            bd=1,
        )

        categories = {
            "Trig (cos, sin, tan)": ["1", "4", "2"],
            "Inverse Trig (asin, acos, atan)": ["9", "10", "11"],
            "Hyperbolic (sinh, cosh, tanh)": ["12", "13", "14"],
            "Log & Exp (log, ln, exp)": ["5", "6", "7"],
            "Roots & Powers (sqrt, cbrt)": ["3", "18"],
            "Rounding & Misc (abs, floor, ceil, round)": ["8", "15", "16", "17"],
            "Angle Conversion (deg, rad)": ["19", "20"],
        }

        cmd_dict = {code: (name, desc) for code, name, desc, _cat in self.COMMANDS}

        for cat_name, codes in categories.items():
            submenu = tk.Menu(
                self.cmd_menu,
                tearoff=0,
                bg=self.PANEL,
                fg=self.TEXT,
                activebackground=self.ACCENT,
                activeforeground=self.TEXT,
                font=("Segoe UI", 10),
            )
            for code in codes:
                name, desc = cmd_dict[code]
                label_text = f"#{code}  {name}(x)  — {desc}"
                submenu.add_command(
                    label=label_text,
                    command=lambda c=code: self._insert_command(c),
                )
            self.cmd_menu.add_cascade(label=cat_name, menu=submenu)

        self.cmd_menu.add_separator()
        self.cmd_menu.add_command(
            label="Just '#' (raw symbol)",
            command=lambda: self._insert("#"),
        )

    def _bind_keys(self):
        self.bind("<Return>", lambda _event: self._calculate())
        self.bind("<KP_Enter>", lambda _event: self._calculate())
        self.bind("<Escape>", lambda _event: self._clear())
        self.bind("<BackSpace>", lambda _event: self._backspace())
        self.bind("<Delete>", lambda _event: self._delete())

    def _handle_action(self, action):
        if action == "EQUALS":
            self._calculate()
        elif action == "CLEAR":
            self._clear()
        elif action == "BACKSPACE":
            self._backspace()
        elif action == "DELETE":
            self._delete()
        elif action == "ANS":
            self._insert_ans()
        elif action == "COMMAND_MENU":
            self._show_command_dropdown()
        else:
            self._insert(action)

    def _show_command_dropdown(self):
        if self._cmd_button is None:
            return

        try:
            x = self._cmd_button.winfo_rootx()
            y = self._cmd_button.winfo_rooty() + self._cmd_button.winfo_height()
            self.cmd_menu.tk_popup(x, y)
        finally:
            self.cmd_menu.grab_release()

    def _insert_command(self, code):
        """Insert a command with parentheses and place cursor directly inside."""
        text = f"#{code}()"
        try:
            if self.display.selection_present():
                pos = self.display.index(tk.SEL_FIRST)
                self.display.delete(tk.SEL_FIRST, tk.SEL_LAST)
            else:
                pos = self.display.index(tk.INSERT)
        except tk.TclError:
            pos = tk.END

        self.display.insert(pos, text)
        # Position cursor inside the parentheses: pos + len(text) - 1
        self.display.icursor(pos + len(text) - 1)
        self.display.focus_set()
        self.status.set(f"Selected #{code}")

    def _insert_ans(self):
        """Insert 'ans' directly into the expression (recognized natively by the C engine)."""
        self._insert("ans")
        if self._last_result:
            self.status.set(f"ans = {self._last_result}")
        else:
            self.status.set("ans (previous C result)")

    def _insert(self, value):
        try:
            if self.display.selection_present():
                self.display.delete(tk.SEL_FIRST, tk.SEL_LAST)
        except tk.TclError:
            pass

        self.display.insert(tk.INSERT, value)
        self.display.focus_set()
        self.status.set("Ready")

    def _clear(self):
        self.expression.set("")
        self.status.set("Ready")
        self.display.focus_set()

    def _backspace(self):
        try:
            if self.display.selection_present():
                self.display.delete(tk.SEL_FIRST, tk.SEL_LAST)
            else:
                position = self.display.index(tk.INSERT)
                if position > 0:
                    self.display.delete(position - 1)
        except tk.TclError:
            pass

        self.display.focus_set()

    def _delete(self):
        try:
            if self.display.selection_present():
                self.display.delete(tk.SEL_FIRST, tk.SEL_LAST)
            else:
                position = self.display.index(tk.INSERT)
                self.display.delete(position)
        except tk.TclError:
            pass

        self.display.focus_set()

    def _calculate(self):
        expression = self.expression.get().strip()

        if not expression:
            self.status.set("Enter an expression")
            return

        value, error = self.backend.evaluate(expression)

        if error:
            self.status.set("Calculation error")
            messagebox.showerror("Calculation Error", error, parent=self)
            return

        self.expression.set(value)
        self._last_result = value
        self.status.set(f"Ans = {value}")
        self.display.icursor(tk.END)

    def _show_commands(self):
        """
        Open a scrollable, non-modal Commands window with all 20 functions.
        The user can freely use the main calculator while this window remains open.
        """
        if (
            self._commands_window is not None
            and self._commands_window.winfo_exists()
        ):
            self._commands_window.lift()
            self._commands_window.focus_force()
            return

        window = tk.Toplevel(self)
        self._commands_window = window
        window.title("Custom Command List (20 Functions)")
        window.geometry("500x560")
        window.minsize(460, 400)
        window.configure(bg=self.BG)

        # Non-modal: NO grab_set()!

        top_bar = tk.Frame(window, bg=self.BG)
        top_bar.pack(fill="x", padx=16, pady=(14, 6))

        tk.Label(
            top_bar,
            text="Command Registry (20 Functions)",
            font=("Segoe UI", 15, "bold"),
            bg=self.BG,
            fg=self.TEXT,
        ).pack(side="left")

        tk.Label(
            window,
            text="Click 'Insert' to add any function to your expression. Use 'ans' for previous result.",
            font=("Segoe UI", 9),
            bg=self.BG,
            fg=self.MUTED,
        ).pack(fill="x", padx=16, pady=(0, 8))

        # Scrollable container
        container = tk.Frame(window, bg=self.PANEL)
        container.pack(fill="both", expand=True, padx=16, pady=4)

        canvas = tk.Canvas(container, bg=self.PANEL, highlightthickness=0)
        scrollbar = tk.Scrollbar(container, orient="vertical", command=canvas.yview)
        scrollable_frame = tk.Frame(canvas, bg=self.PANEL)

        scrollable_frame.bind(
            "<Configure>",
            lambda e: canvas.configure(scrollregion=canvas.bbox("all")),
        )

        canvas.create_window((0, 0), window=scrollable_frame, anchor="nw")
        canvas.configure(yscrollcommand=scrollbar.set)

        canvas.pack(side="left", fill="both", expand=True)
        scrollbar.pack(side="right", fill="y")

        # Mousewheel binding
        def _on_mousewheel(event):
            canvas.yview_scroll(int(-1 * (event.delta / 120)), "units")

        canvas.bind_all("<MouseWheel>", _on_mousewheel)

        for code, name, desc, cat in self.COMMANDS:
            row_frame = tk.Frame(scrollable_frame, bg=self.PANEL)
            row_frame.pack(fill="x", padx=10, pady=3)

            tk.Label(
                row_frame,
                text=f"#{code}(x)",
                font=("Cascadia Mono", 10, "bold"),
                bg=self.PANEL,
                fg=self.TEXT,
                width=8,
                anchor="w",
            ).pack(side="left", padx=4)

            tk.Label(
                row_frame,
                text=f"{name}(x) — {desc}",
                font=("Segoe UI", 9),
                bg=self.PANEL,
                fg=self.MUTED,
                anchor="w",
            ).pack(side="left", expand=True, fill="x", padx=4)

            tk.Button(
                row_frame,
                text="Insert",
                command=lambda c=code: self._insert_command(c),
                font=("Segoe UI", 8, "bold"),
                bg=self.BUTTON,
                fg=self.ACCENT,
                activebackground=self.BUTTON_HOVER,
                activeforeground=self.TEXT,
                relief="flat",
                cursor="hand2",
                padx=8,
                pady=2,
            ).pack(side="right", padx=4)

        bottom_bar = tk.Frame(window, bg=self.BG)
        bottom_bar.pack(fill="x", padx=16, pady=(8, 12))

        tk.Label(
            bottom_bar,
            text="Constants: pi, e, ans (C native register)",
            font=("Segoe UI", 9),
            bg=self.BG,
            fg=self.MUTED,
        ).pack(side="left")

        tk.Button(
            bottom_bar,
            text="Close",
            command=window.destroy,
            font=("Segoe UI", 9, "bold"),
            bg=self.PANEL,
            fg=self.TEXT,
            activebackground=self.BUTTON_HOVER,
            activeforeground=self.TEXT,
            relief="flat",
            padx=20,
            pady=4,
            cursor="hand2",
        ).pack(side="right")

    def _on_close(self):
        self.backend.close()
        self.destroy()


if __name__ == "__main__":
    app = ScientificCalculatorGUI()
    app.mainloop()
