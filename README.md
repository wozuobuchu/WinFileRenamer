# WinFileRenamer

A Windows GUI executable to rename files in batches, with custom expression support.



---

## English Instructions

### Features
- Select one or more files to batch rename.
- Build new file names using an expression: supports strings, numbers, index variable, original file name, mathematical operations, and simple formatting.
- Preview and process renaming interactively via a user-friendly interface.
- Safe: will not overwrite existing files, with error reporting.

### Usage

1. **Open File(s)**:  
   Use the `File -> Open` menu to select one or more files you want to rename.

2. **Clear Selection**:  
   Use `File -> Clear` to remove all currently selected files.

3. **Build Rename Expression**:
   - Use the `Edit` menu’s push options to build your rename rule:
     - Push Wstring : insert a string, type in the input box.
     - Push Number : insert a number, type in the input box.
     - Push Index : adds the index of the file (starts from 0).
     - Push OriginFileName : adds the original file name (no extension).
     - Push MinimunNumLength : pads an integer to minimum length, type the length, it should multiply a number.
     - Parentheses, and mathematical operators (+, -, *, /) are supported.
     - Use Delete to remove the last element, or Clear to reset the expression.

4. **Submit**:  
   Use `File -> Submit` to execute renaming.  
   If success, a dialog with the result will be shown. If errors occur (such as name exists or expression error), an error message will appear.
   
5. **Exit**:  
   Use `Option -> Exit` to close the program.

#### Example Expression
- Add index to all filenames, padded to 3 digits:  
  `INDEX`, `*`, `NUM_FORMAT_3`, `+`, `OFNAME`  

