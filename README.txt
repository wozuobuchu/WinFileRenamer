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
     - Push Wstring [STR]: insert a string, type in the input box.
     - Push Number [INT]: insert a number, type in the input box.
     - Push Index [VAR]: adds the index of the file (starts from 0).
     - Push OriginFileName [VAR]: adds the original file name (no extension).
     - Push MinimunNumLength [FMT*INT -> STR]: pads an integer to minimum length, type the length.
     - Parentheses, and mathematical operators (+, -, *, /) are supported.
     - Use Delete to remove the last element, or Clear to reset the expression.

4. **Submit**:  
   Use `File -> Submit` to execute renaming.  
   If success, a dialog with the result will be shown. If errors occur (such as name exists or expression error), an error message will appear.
   
5. **Exit**:  
   Use `Option -> Exit` to close the program.

#### Example Expression
- Add index to all filenames, padded to 3 digits:  
  `INDEX`, `NUM_FORMAT_3`, `*`, `OFNAME`, `+`

---

## 中文使用说明

### 功能特色
- 支持批量选择文件进行重命名。
- 通过表达式自定义新文件名：支持字符串、数字、序号变量、原文件名、数学运算与简单格式化。
- 可视化界面，实时预览表达式内容与批量处理。
- 安全：不会覆盖已存在文件，出现错误会有提示。

### 使用方法

1. **打开文件**：  
   点击 `File -> Open` 菜单，选择要重命名的一个或多个文件。

2. **清空列表**：  
   点击 `File -> Clear` 清除所有已选文件。

3. **构建重命名表达式**：  
   - 在 `Edit` 菜单下用 push 相关项添加所需元素：
     - Push Wstring [STR]：插入字符串，输入文字后点击添加。
     - Push Number [INT]：插入数字，输入后点击添加。
     - Push Index [VAR]：添加文件顺序号（从0开始）。
     - Push OriginFileName [VAR]：添加原始文件名（不带后缀）。
     - Push MinimunNumLength [FMT*INT -> STR]：将数字最少填充为指定长度，用于补零。
     - 可插入括号与加减乘除运算符。
     - 用 Delete 删除最后一项，Clear 清空表达式。

4. **提交执行**：  
   点击 `File -> Submit` 启动重命名。  
   操作成功会弹窗提示结果。如有文件名冲突、表达式错误等，会显示错误信息。

5. **退出程序**：  
   通过 `Option -> Exit` 关闭程序。

#### 示例表达式
- 批量为文件名加上三位数序号：  
  依次添加 `INDEX`，`NUM_FORMAT_3`，`*`，`OFNAME`，`+`

---

### Note / 注意事项

- Supported only in Windows environments with GUI.
- Expressions must be valid; illegal input or conflicting filenames will abort the operation with an error dialog.
- 文件名表达式需拼写正确，重名或非法输入会中止并提示错误。

---