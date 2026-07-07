#include "theme.h"

// ===== 亮主题 =====
Theme Theme::lightTheme()
{
    Theme t;
    t.name = "Light";

    // 编辑器
    t.editorBackground = QColor("#FFFFFF");     // 白色
    t.editorForeground = QColor("#000000");     // 黑色

    // 行号栏
    t.lineNumberBackground = QColor("#F0F0F0");
    t.lineNumberForeground = QColor("#808080");

    // 当前行
    t.currentLineHighlight = QColor("#FFFACD"); // 淡黄

    // 括号
    t.bracketMatchColor = QColor("#B3D9FF");    // 淡蓝

    // 语法高亮（亮主题配色）
    t.keywordColor = QColor("#0000FF");         // 深蓝
    t.qtClassColor = QColor("#008080");         // 深青
    t.preprocessorColor = QColor("#800080");    // 紫色
    t.numberColor = QColor("#098658");          // 深绿
    t.stringColor = QColor("#A31515");          // 深红
    t.functionColor = QColor("#795E26");        // 深黄
    t.commentColor = QColor("#008000");         // 绿色

    return t;
}

// ===== 暗主题（VSCode 风格）=====
Theme Theme::darkTheme()
{
    Theme t;
    t.name = "Dark";

    // 编辑器
    t.editorBackground = QColor("#1E1E1E");     // 深灰
    t.editorForeground = QColor("#D4D4D4");     // 浅灰

    // 行号栏
    t.lineNumberBackground = QColor("#252526");
    t.lineNumberForeground = QColor("#858585");

    // 当前行
    t.currentLineHighlight = QColor("#2A2A2A"); // 稍亮的灰

    // 括号
    t.bracketMatchColor = QColor("#264F78");    // 深蓝

    // 语法高亮（暗主题配色 - VSCode Dark+）
    t.keywordColor = QColor("#569CD6");         // 蓝色
    t.qtClassColor = QColor("#4EC9B0");         // 青色
    t.preprocessorColor = QColor("#C586C0");    // 紫色
    t.numberColor = QColor("#B5CEA8");          // 浅绿
    t.stringColor = QColor("#CE9178");          // 橙色
    t.functionColor = QColor("#DCDCAA");        // 淡黄
    t.commentColor = QColor("#6A9955");         // 灰绿

    return t;
}

// 🆕 根据名字返回对应颜色
QColor Theme::colorByName(const QString &name) const
{
    if (name == "keyword") return keywordColor;
    if (name == "qtClass") return qtClassColor;
    if (name == "preprocessor") return preprocessorColor;
    if (name == "number") return numberColor;
    if (name == "string") return stringColor;
    if (name == "function") return functionColor;
    if (name == "comment") return commentColor;
    if (name == "foreground") return editorForeground;
    // 默认返回前景色
    return editorForeground;
}