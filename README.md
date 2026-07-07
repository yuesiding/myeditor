# MyEditor 📝

一个用 Qt 5 + C++ 开发的轻量级多语言代码编辑器，支持语法高亮、代码折叠、多标签编辑等功能。

## ✨ 主要特性

### 编辑功能
- 📝 **多标签页编辑**：同时打开和编辑多个文件
- 🎨 **语法高亮**：内置支持 C++、Python、JSON
- 📐 **行号显示** & 当前行高亮
- 🔗 **括号匹配**：光标旁边的括号自动高亮
- 📂 **代码折叠**：折叠函数/类，聚焦当前工作
- 🔍 **查找和替换**：支持大小写、全词、正则表达式
- ↩️ **自动缩进**：按上一行缩进级别自动对齐

### 界面
- 🌗 **亮/暗主题**：可自由切换
- 🌳 **文件树侧边栏**：像 VSCode 一样浏览项目
- 🛠️ **工具栏**：常用操作一键触发
- 📊 **专业状态栏**：显示行列号、字符数、折叠数

### 用户体验
- 💾 **会话恢复**：重启后自动打开上次的文件
- 📌 **最近文件**：快速访问最近打开的 10 个文件
- 🖱️ **拖拽打开**：拖文件到窗口即可打开
- ⌨️ **命令行支持**：`myeditor.exe file.cpp`
- 🔎 **字体缩放**：Ctrl + 滚轮

### 可扩展性
- 📦 **数据驱动**：语法规则用 JSON 定义，加新语言无需修改代码
- ⚙️ **首选项对话框**：字体、缩进、主题等可自定义

## 📸 截图

（这里以后你可以添加截图）

## 🛠️ 技术栈

- **框架**：Qt 5.14.2
- **语言**：C++17
- **构建系统**：CMake 3.16+
- **编译器**：MinGW 7.3.0 (64-bit)

## 🚀 构建

### 依赖
- Qt 5.14.2（MinGW 64-bit）
- CMake 3.16 或更高版本
- MinGW 7.3.0 或兼容版本

### 编译步骤

```bash
# 克隆仓库
git clone https://github.com/yuesiding/myeditor.git
cd myeditor

# 使用 CMake 构建
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make


📦 使用发布版
如果你不想自己编译，可以直接下载 Releases 里的 ZIP 包，解压后双击 CodeEditor.exe 即可运行。

🎓 项目背景
这是一个学习 Qt 的完整项目，从零开始一步步实现。涉及的核心技术：

QMainWindow / QPlainTextEdit / QTabWidget
QSyntaxHighlighter + QRegularExpression
单例模式（ThemeManager, SyntaxManager）
观察者模式（信号槽）
MVC 模式（QFileSystemModel + QTreeView）
配置驱动的可扩展架构（JSON 语法定义）
📁 项目结构
myeditor/
├── main.cpp                    # 程序入口
├── mainwindow.{h,cpp}          # 主窗口
├── editorwidget.{h,cpp}        # 编辑器控件
├── highlighter.{h,cpp}         # 高亮器基类
├── generichighlighter.{h,cpp}  # 通用高亮器（数据驱动）
├── syntaxdefinition.{h,cpp}    # 语法定义
├── syntaxmanager.{h,cpp}       # 语法管理器（单例）
├── theme.{h,cpp}               # 主题
├── thememanager.{h,cpp}        # 主题管理器（单例）
├── findreplacedialog.{h,cpp}   # 查找替换对话框
├── preferencesdialog.{h,cpp}   # 首选项对话框
├── filetreewidget.{h,cpp}      # 文件树
├── syntax/                     # 语法定义 JSON
│   ├── cpp.json
│   ├── python.json
│   └── json.json
└── CMakeLists.txt

📝 支持添加新语言
只需在 syntax/ 目录下添加一个 JSON 文件即可：

JSON

{
    "name": "JavaScript",
    "extensions": ["js"],
    "keywords": ["function", "var", "let", "const", "if", "else", ...],
    "singleLineComment": "//",
    "multiLineCommentStart": "/*",
    "multiLineCommentEnd": "*/",
    ...
}

📄 License
MIT License

🙏 致谢
感谢 Qt 团队提供如此强大的 GUI 框架。

text


