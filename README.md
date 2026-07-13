<div align="center">

# 📝 MyEditor

**一个用 Qt 5 + C++ 开发的轻量级代码编辑器**

*内置多语言语法高亮、任务清单、AI 智能助手*

![Qt](https://img.shields.io/badge/Qt-5.14.2-41CD52?logo=qt&logoColor=white)
![C++](https://img.shields.io/badge/C++-17-00599C?logo=cplusplus&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.16+-064F8C?logo=cmake&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?logo=windows&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-yellow)

</div>

---

## ✨ 主要特性

### 📝 编辑功能

- 📑 **多标签页编辑** — 同时打开和编辑多个文件
- 🎨 **实时语法高亮** — 内置支持 C++、Python、JSON 三种语言
- 📐 **行号显示** 与当前行高亮
- 🔗 **括号匹配** — 光标旁的括号自动高亮
- 📂 **代码折叠** — 折叠函数/类，聚焦当前工作
- 🔍 **查找与替换** — 支持大小写、全词、正则表达式
- ↩️ **自动缩进** — 按上一行缩进级别自动对齐

### 🎨 界面

- 🌗 **亮/暗主题** — 一键切换
- 🛠️ **工具栏** — 常用操作一键触发
- 📊 **专业状态栏** — 显示行列号、字符数、折叠数
- ⚙️ **首选项对话框** — 字体、缩进、主题可自定义

### 📋 任务管理

- ✅ **内置任务清单** — 右侧侧边栏管理待办事项
- 🎯 **优先级标记** — 高/中/低三级优先级
- 🔍 **智能筛选** — 按全部/未完成/已完成筛选
- 💾 **自动持久化** — 任务数据自动保存

### 🤖 AI 智能助手

- 💬 **AI 对话** — 内置 DeepSeek AI 助手，可自由问答
- 📝 **代码解释** — 选中代码右键，AI 秒懂
- 🚀 **代码优化** — AI 给出优化建议
- 🐛 **AI 找 Bug** — 智能分析潜在问题
- 💡 **代码翻译** — 一键转换为其他语言

### 💾 数据管理

- 💾 **会话恢复** — 重启后自动打开上次的文件
- ❓ **未保存提示** — 关闭时智能提醒

### 🔧 可扩展性

- 📦 **数据驱动** — 语法规则用 JSON 定义，加新语言无需修改代码

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

### 从源码编译

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

### 使用 AI 助手（可选）

如需使用 AI 助手功能：

1. 注册 [DeepSeek](https://platform.deepseek.com) 账号（新用户送免费额度）
2. 在 API keys 页面创建 API key
3. 打开编辑器 → AI 助手侧边栏 → 点击 "🔑 设置 API Key"
4. 粘贴 API key 并保存

---

## 🎓 项目背景

这是一个学习 Qt 的完整项目，从零开始一步步实现。

### 涉及的核心技术

- **Qt 核心类**：`QMainWindow`、`QPlainTextEdit`、`QTabWidget`
- **语法高亮**：`QSyntaxHighlighter` + `QRegularExpression`
- **网络通信**：`QNetworkAccessManager`（AI 助手）
- **JSON 解析**：`QJsonDocument`（语法定义 + 任务持久化）
- **数据持久化**：`QSettings`（会话恢复 + 用户设置）

### 设计模式

- **单例模式**：`ThemeManager`、`SyntaxManager`
- **观察者模式**：Qt 信号槽机制
- **模板方法模式**：`Highlighter` 基类 + `GenericHighlighter` 子类
- **数据驱动**：JSON 配置替代硬编码

---

## 📁 项目结构

```
myeditor/
├── main.cpp                        # 程序入口
├── mainwindow.{h,cpp}              # 主窗口
├── editorwidget.{h,cpp}            # 编辑器控件（含行号栏、折叠区）
├── highlighter.{h,cpp}             # 高亮器基类
├── generichighlighter.{h,cpp}      # 通用高亮器（数据驱动）
├── syntaxdefinition.{h,cpp}        # 语法定义
├── syntaxmanager.{h,cpp}           # 语法管理器（单例）
├── theme.{h,cpp}                   # 主题
├── thememanager.{h,cpp}            # 主题管理器（单例）
├── findreplacedialog.{h,cpp}      # 查找替换对话框
├── preferencesdialog.{h,cpp}      # 首选项对话框
├── tasklistwidget.{h,cpp}          # 任务清单
├── task.h                          # 任务数据结构
├── aiassistantwidget.{h,cpp}      # AI 助手
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

### 文件操作

| 快捷键 | 功能 |
|--------|------|
| `Ctrl + N` | 新建文件 |
| `Ctrl + O` | 打开文件 |
| `Ctrl + S` | 保存文件 |
| `Ctrl + Shift + S` | 另存为 |
| `Ctrl + Q` | 退出 |

### 编辑操作

| 快捷键 | 功能 |
|--------|------|
| `Ctrl + Z` | 撤销 |
| `Ctrl + Y` | 重做 |
| `Ctrl + X / C / V` | 剪切 / 复制 / 粘贴 |
| `Ctrl + F` | 查找与替换 |

### 视图操作

| 快捷键 | 功能 |
|--------|------|
| `Ctrl + T` | 显示/隐藏任务清单 |
| `Ctrl + I` | 显示/隐藏 AI 助手 |
| `Ctrl + ,` | 打开首选项 |
| `Ctrl + +` | 放大字体 |
| `Ctrl + -` | 缩小字体 |
| `Ctrl + 0` | 重置字体大小 |
| `Ctrl + K, Ctrl + 0` | 折叠所有 |
| `Ctrl + K, Ctrl + J` | 展开所有 |

---

## 🎯 主要功能演示

### 语法高亮
打开 `.cpp` / `.py` / `.json` 文件，编辑器会**自动识别**并应用对应的高亮规则。

### AI 助手
- 在编辑器中**选中一段代码**
- **右键** → 选择 "🤖 AI 解释这段代码" / "🚀 AI 优化" / "🐛 AI 找 Bug"
- AI 助手侧边栏自动弹出并显示分析结果

### 任务清单
- 右侧 "📋 任务清单" 面板
- 输入任务 → 回车添加
- 点击复选框标记完成
- 右键任务可修改优先级

---

## 📄 License

本项目基于 [MIT License](LICENSE) 开源。

---

## 🙏 致谢

- 感谢 [Qt](https://www.qt.io/) 团队提供如此强大的 GUI 框架
- 感谢 [DeepSeek](https://platform.deepseek.com) 提供高质量的 AI API 服务
- 感谢所有开源社区的贡献者

---

<div align="center">

**⭐ 如果这个项目对你有帮助，欢迎 Star 支持！**

Made with ❤️ by [yuesiding](https://github.com/yuesiding)

</div>