<div align="center">

# 📝 MyEditor

**一个用 Qt 5 + C++ 开发的轻量级多语言代码编辑器**

*支持语法高亮、代码折叠、多标签编辑等专业功能*

![Qt](https://img.shields.io/badge/Qt-5.14.2-41CD52?logo=qt&logoColor=white)
![C++](https://img.shields.io/badge/C++-17-00599C?logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.16+-064F8C?logo=cmake&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?logo=windows&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-yellow)
![Version](https://img.shields.io/badge/Version-4.0.0-brightgreen)
</div>

---

## ✨ 主要特性

### 📝 编辑功能

- 📑 **多标签页编辑** — 同时打开和编辑多个文件
- 🎨 **语法高亮** — 内置支持 C++、Python、JSON
- 📐 **行号显示** 与当前行高亮
- 🔗 **括号匹配** — 光标旁的括号自动高亮
- 📂 **代码折叠** — 折叠函数/类，聚焦当前工作
- 🔍 **查找与替换** — 支持大小写、全词、正则表达式
- ↩️ **自动缩进** — 按上一行缩进级别自动对齐

### 🎨 界面

- 🌗 **亮/暗主题** — 一键切换
- 🌳 **文件树侧边栏** — 像 VSCode 一样浏览项目
- 🛠️ **工具栏** — 常用操作一键触发
- 📊 **专业状态栏** — 显示行列号、字符数、折叠数

### 💡 用户体验

- 💾 **会话恢复** — 重启后自动打开上次的文件
- 📌 **最近文件** — 快速访问最近打开的 10 个文件
- 🖱️ **拖拽打开** — 拖文件到窗口即可打开
- ⌨️ **命令行支持** — `myeditor.exe file.cpp`
- 🔎 **字体缩放** — Ctrl + 鼠标滚轮
- 
### 📋 任务管理

- ✅ **内置任务清单** — 右侧侧边栏管理待办事项
- 🎯 **优先级标记** — 高/中/低三级优先级，不同颜色区分
- 🔍 **智能筛选** — 按全部/未完成/已完成筛选
- 💾 **自动持久化** — 所有任务自动保存，重启后恢复
- 📊 **实时统计** — 状态栏显示待办和完成数
- 🖱️ **便捷操作** — 双击编辑，右键复制/置顶/置底

### 🤖 AI 智能助手

- 💬 **AI 对话**：内置 DeepSeek AI 助手，可自由问答
- 📝 **代码解释**：选中代码右键，AI 秒懂
- 🚀 **代码优化**：AI 给出优化建议
- 🐛 **AI 找 Bug**：智能分析潜在问题
- 💡 **代码翻译**：一键转换为 Python
- 📖 **Markdown 渲染**：AI 回复漂亮排版

### 🚀 代码运行

- ▶️ **一键运行**：F5 执行当前文件
- 🔨 **自动编译**：C++/C 文件自动编译再运行
- 🖥️ **内置终端**：底部面板显示程序输出
- ⏹️ **随时中断**：Shift+F5 停止运行
- 🎨 **多语言支持**：Python、JavaScript、C++、C、Batch

### 🎨 视觉体验

- ⚡ **命令面板**：Ctrl+Shift+P 快速执行所有命令
- 🗺️ **迷你地图**：右侧代码缩略图
- 🌗 **亮/暗主题**：一键切换

### 🧠 思维导图（v4.0 新增！）

- 🎨 **可视化画布** — 无限画布，滚轮缩放，中键拖动
- 🎯 **节点系统** — 中心主题 + 子节点，贝塞尔曲线连接
- ⌨️ **快捷操作** — Tab 添加子节点，Enter 添加同级
- ✨ **AI 联动** — 选中节点，AI 自动生成子话题！
- 🎨 **样式定制** — 颜色、形状、可折叠子分支
- 💾 **完整持久化** — 保存为 .qmind (JSON) 文件
- 🖼️ **导出 PNG** — 一键导出为高清图片
- ↶ **撤销重做** — 完整的命令模式
- 🌗 **主题适配** — 亮/暗主题下都好看

### 🔧 可扩展性

- 📦 **数据驱动** — 语法规则用 JSON 定义，加新语言无需修改代码
- ⚙️ **首选项对话框** — 字体、缩进、主题等可自定义

---

## 📸 截图

> 待补充

---

## 🛠️ 技术栈

| 类别 | 技术 |
|------|------|
| **框架** | Qt 5.14.2 |
| **语言** | C++17 |
| **构建系统** | CMake 3.16+ |
| **编译器** | MinGW 7.3.0 (64-bit) |
| **AI 服务** | DeepSeek API |
| **网络** | Qt Network + OpenSSL |
---

## 🚀 快速开始

### 方式一：直接使用发布版

如果你不想自己编译，可以直接下载 [Releases](https://github.com/yuesiding/myeditor/releases) 里的 ZIP 包，解压后双击 `CodeEditor.exe` 即可运行。

### 方式二：从源码编译

**依赖**：

- Qt 5.14.2（MinGW 64-bit 版本）
- CMake 3.16 或更高版本
- MinGW 7.3.0 或兼容版本

**编译步骤**：

```bash
# 1. 克隆仓库
git clone https://github.com/yuesiding/myeditor.git
cd myeditor

# 2. 创建构建目录
mkdir build
cd build

# 3. 使用 CMake 生成 Makefile
cmake -G "MinGW Makefiles" ..

# 4. 编译
mingw32-make
```

编译完成后，`build/` 目录下会生成 `CodeEditor.exe`。

---

## 🎓 项目背景

这是一个学习 Qt 的完整项目，从零开始一步步实现。

### 涉及的核心技术

- **Qt 核心类**：`QMainWindow`、`QPlainTextEdit`、`QTabWidget`
- **语法高亮**：`QSyntaxHighlighter` + `QRegularExpression`
- **单例模式**：`ThemeManager`、`SyntaxManager`
- **观察者模式**：信号槽机制
- **MVC 模式**：`QFileSystemModel` + `QTreeView`
- **配置驱动**：JSON 语法定义，可扩展架构

---

## 📁 项目结构

```
myeditor/
├── main.cpp                        # 程序入口
├── mainwindow.{h,cpp}              # 主窗口
├── editorwidget.{h,cpp}            # 编辑器控件
├── highlighter.{h,cpp}             # 高亮器基类
├── generichighlighter.{h,cpp}      # 通用高亮器（数据驱动）
├── syntaxdefinition.{h,cpp}        # 语法定义
├── syntaxmanager.{h,cpp}           # 语法管理器（单例）
├── theme.{h,cpp}                   # 主题
├── thememanager.{h,cpp}            # 主题管理器（单例）
├── findreplacedialog.{h,cpp}       # 查找替换对话框
├── preferencesdialog.{h,cpp}       # 首选项对话框
├── filetreewidget.{h,cpp}          # 文件树
├── syntax/                         # 语法定义 JSON
│   ├── cpp.json
│   ├── python.json
│   └── json.json
├── CMakeLists.txt                  # 构建配置
├── README.md                       # 项目说明
└── LICENSE                         # 开源许可证
```

---

## 🌐 添加新语言支持

只需在 `syntax/` 目录下添加一个 JSON 文件，无需修改任何代码：

```json
{
    "name": "JavaScript",
    "extensions": ["js", "mjs"],
    "keywords": [
        "function", "var", "let", "const",
        "if", "else", "for", "while", "return"
    ],
    "singleLineComment": "//",
    "multiLineCommentStart": "/*",
    "multiLineCommentEnd": "*/",
    "stringPatterns": [
        "\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\"",
        "'[^'\\\\]*(?:\\\\.[^'\\\\]*)*'"
    ],
    "numberPattern": "\\b[0-9]+\\.?[0-9]*\\b",
    "functionPattern": "\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()"
}
```

---

## ⌨️ 快捷键

| 快捷键 | 功能 |
|--------|------|
| `Ctrl + N` | 新建文件 |
| `Ctrl + O` | 打开文件 |
| `Ctrl + S` | 保存文件 |
| `Ctrl + Shift + S` | 另存为 |
| `Ctrl + F` | 查找与替换 |
| `Ctrl + Z` | 撤销 |
| `Ctrl + Y` | 重做 |
| `Ctrl + B` | 显示/隐藏文件浏览器 |
| `Ctrl + ,` | 打开首选项 |
| `Ctrl + +` | 放大字体 |
| `Ctrl + -` | 缩小字体 |
| `Ctrl + 0` | 重置字体大小 |
| `Ctrl + K, Ctrl + 0` | 折叠所有 |
| `Ctrl + K, Ctrl + J` | 展开所有 |
| `Ctrl + T` | 显示/隐藏任务清单 |
| `Ctrl + Shift + P` | 打开命令面板 |
| `Ctrl + I` | 显示/隐藏 AI 助手 |
| `F5` | 运行当前文件 |
| `Shift + F5` | 停止运行 |
| `` Ctrl + ` `` | 显示/隐藏终端 |
### 🧠 思维导图模式

| `Tab` | 添加子节点 |
| `Enter` | 添加同级节点 |
| `Delete` | 删除选中节点 |
| `F2` | 编辑节点 |
| `Ctrl+A` | 全选 |
| `Ctrl+Z` | 撤销 |
| `Ctrl+Y` | 重做 |
| `+` / `-` | 缩放 |
| `0` | 重置视图 |
---

## 📄 License

本项目基于 [MIT License](LICENSE) 开源。

---

## 🙏 致谢

- 感谢 [Qt](https://www.qt.io/) 团队提供如此强大的 GUI 框架
- 感谢所有开源社区的贡献者

---

<div align="center">

**⭐ 如果这个项目对你有帮助，欢迎 Star 支持！**

Made with ❤️ by [yuesiding](https://github.com/yuesiding)

</div>