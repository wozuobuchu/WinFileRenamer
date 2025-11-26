# WinFileRenamer

A batch file renaming tool based on **expressions/formulas**, implemented with a native Win32 GUI.
It renames multiple selected files using a unified rule.

`⚠️ This README is AI-generated and may contain inaccuracies.`

---

## 0. Screenshot Preview

* Align anime subtitle filenames in bulk for automatic subtitle loading in media players

<img src="./img/cover.png" alt="cover"> <br>

---

## 1. Introduction

* **Platform**: Windows (Win32 API + Common Controls)
* **UI Structure**:

  * Top: `Selected Files` list showing the full paths of selected files
  * Middle: `Expression Preview`, read-only, showing the currently constructed renaming expression
  * Bottom: `Input` textbox, used to enter temporary text/numbers and "push" them into the expression through the menu
* **Core Concept**:
  The program includes a simple **expression engine**, where strings, integers, variables (index, original filename), parentheses, and operators are unified as `Element` objects.
  These are converted into Reverse Polish Notation (RPN) and evaluated to produce the final target path string.
* **Batch Execution**:
  For each file in the list, the program uses its index and original filename as variables, evaluates the same expression, and generates a target path.
  Renaming is performed using `std::filesystem::rename`.
* **Safety Mechanisms**:

  * Target file exists → Abort with error
  * Original file missing → Abort
  * Invalid expression (mismatched parentheses, type errors, missing operands, etc.) → Abort with error popup

---

## 2. Usage

### 2.1 Launching the Program

Run `WinFileRenamer.exe`.
`WinMain` registers the window class and creates the main window `WinFileRenamer`.

### 2.2 Selecting Files

Menu: **File → Open**

* Opens the standard file-select dialog; supports selecting one or multiple files.
* Selected files are appended to the `Selected Files` list along with their full paths.

Clear list: **File → Clear**

* Removes all entries and clears the internal path cache.

> ⚠️ File indices start from **0** in the order they appear in the list: 0, 1, 2, 3, …

### 2.3 Building Expressions

Expressions are **not** typed directly into the Input textbox.
Instead, construct them step-by-step using the **Edit menu**.

Recommended workflow:

1. Type text/number into `Input`
2. Select the corresponding `Push XXX` option in the **Edit menu**
3. The preview updates to show the full expression

**Common Edit Menu Commands:**

* **Push Wstring**
  Inserts the `Input` content as a **string literal**
  Appears as `"text"` in the preview.

* **Push Number**
  Parses `Input` as a 64-bit integer.
  Invalid input triggers an error popup.

* **Push Index**
  Inserts the variable **INDEX** (0,1,2,…).

* **Push OriginFileName**
  Inserts **OFNAME**, the **original filename (with extension, without path)**.

* **Push MinimunNumLength**
  Parses `Input` as integer `n`, creating a `NUM_FORMAT_n` descriptor (`Int64_Format`).
  Designed to be multiplied with an integer to generate zero-padded numbers.

* **Push ( / Push )**
  Inserts parentheses.

* **Push + / Push - / Push * / Push /**
  Inserts arithmetic operators.

* **Delete**
  Removes the last element in the expression.

* **Clear**
  Clears the entire expression.

Examples of expressions shown in the preview:

```
"img_" + NUM_FORMAT_4 * INDEX + ".jpg"
```

```
"text_" + ( 2 * INDEX + 1 ) + ".txt"
```

### 2.4 Executing Rename

When the expression is complete:

Menu: **File → Submit Rename**

A background thread performs:

1. Convert expression to RPN
2. For each file:

   * Replace `INDEX` with file index
   * Replace `OFNAME` with its original filename
3. Evaluate the expression to get the final target path
4. Call `std::filesystem::rename(oldPath, newPath)`

If any step fails (invalid expression, missing file, target exists, permission issue, etc.), the process aborts with an error message.
If all succeed, a success dialog appears.

> ⚠️ Important:
> The expression’s evaluation result is the **complete final path**, not just the filename.
>
> * If you want to rename in the original directory, you must explicitly include it:
>   `"D:\Data\" + ...`
> * If you output only a filename, the path becomes relative to the **process working directory**, which is usually *not* the file’s directory.

---

## 3. Expression Rules

### 3.1 Data Types

* **String (`Str`)**
  Created via `Push Wstring`.

* **Integer (`Int64`)**
  Created via `Push Number`.

* **Format Descriptor (`Int64_Format`)**
  Shown as `NUM_FORMAT_n`.
  Generates zero-padded numbers when multiplied with integers.

* **Variables (`Var`)**

  * `INDEX`: index in the list
  * `OFNAME`: original filename (with extension)

* **Parentheses**
  `(` and `)` control precedence.

* **Operators (`Int64Opt`)**
  `+`, `-`, `*`, `/`

### 3.2 Operator Semantics & Precedence

* `*` and `/` take priority over `+` and `-`
* Parentheses override precedence

Operation behaviors:

1. **Addition `+`**

   * int + int → integer add
   * string involved → string concatenation
     (integers auto-converted to string)

2. **Subtraction `-`**

   * integer only

3. **Multiplication `*`**

   * int × int → integer multiply
   * int × format or format × int → zero-padded string

     * `NUM_FORMAT_4 * 5` → `"0005"`
     * if `INDEX = 12`: `NUM_FORMAT_3 * INDEX` → `"012"`

4. **Division `/`**

   * integer division
   * divide by 0 → returns very large integer (`0x7fffffffffffffff`)

### 3.3 Variable Replacement

Before evaluation:

* `INDEX` → replaced with integer index
* `OFNAME` → replaced with `Str` from file’s `filename()`

### 3.4 Results & Errors

After RPN evaluation:

* Result must be a single element:

  * integer → converted to string
  * string → used directly
* Errors include:

  * mismatched parentheses
  * missing operands/operators
  * type mismatch
  * unresolved variables

Errors are caught and displayed through message boxes.

---

## 4. Usage Examples

> All examples assume files are in `D:\Data\`.
> Replace the directory with your actual path.

### Example 1: Rename to `img_0000.jpg` format

Desired output:

```
img_0000.jpg
img_0001.jpg
img_0002.jpg
...
```

Expression:

```
"img_" + NUM_FORMAT_4 * INDEX + ".jpg"
```

Steps:

1. Input `img_` → Push Wstring
2. Push +
3. Input `4` → Push MinimunNumLength
4. Push *
5. Push Index
6. Push +
7. Input `.jpg` → Push Wstring

---

### Example 2: Numbering from 1 instead of 0

```
file_0001.txt
file_0002.txt
...
```

Expression:

```
"D:\Data\" + "file_" + (NUM_FORMAT_4 * (INDEX + 1)) + ".txt"
```

Key steps:

* Build `(INDEX + 1)`
* Combine with `NUM_FORMAT_4 * (...)`
* Concatenate directory, prefix, suffix

---

### Example 3: Move files into grouped subdirectories (directories must exist)

Pre-create:

```
Group_00\
Group_01\
Group_02\
...
```

Goal:

```
Files 0–9   → D:\Data\Group_00\filename
Files 10–19 → D:\Data\Group_01\filename
```

Expression:

```
"D:\Data\Group_" + (NUM_FORMAT_2 * (INDEX / 10)) + "\" + OFNAME
```

Explanation:

* `INDEX / 10` gives group ID
* `NUM_FORMAT_2 * (...)` pads it: `"00"`, `"01"`
* Final result becomes: `D:\Data\Group_00\xxx.ext`

> ⚠️ Directories must already exist.
> The program will NOT auto-create them.

---

## 5. Known Limitations

1. **Windows-only** (Win32 API GUI)

2. **No undo/rollback**
   Once renamed, cannot restore automatically.
   Test with sample files first.

3. **Limited expression functions**
   Supported: literals, integers, INDEX, OFNAME, NUM_FORMAT_n, + - * /, parentheses
   Not supported: conditions, regex, substring operations, replacements, etc.

4. **Cannot auto-create directories**
   Missing target paths cause rename failure.

5. **Target filename must not already exist**
   Conflicts cause abort.

6. **Generated paths must be valid Windows paths**
   Illegal characters like `<>:"/\|?*` cause failure—no sanitization performed.

7. **Only one rename operation at a time**
   A running task blocks new submissions.

Within these constraints, WinFileRenamer effectively handles most common renaming operations such as prefix/suffix insertion, sequential numbering, and simple grouped moves.

