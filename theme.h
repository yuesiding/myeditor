#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QString>

// 主题配置：一个 Theme 对象就是一套完整的配色
struct Theme {
    QString name;

    // 编辑器颜色
    QColor editorBackground;
    QColor editorForeground;

    // 行号栏
    QColor lineNumberBackground;
    QColor lineNumberForeground;

    // 当前行高亮
    QColor currentLineHighlight;

    // 括号匹配
    QColor bracketMatchColor;

    // 语法高亮
    QColor keywordColor;      // 关键字
    QColor qtClassColor;      // Qt 类
    QColor preprocessorColor; // 预处理
    QColor numberColor;       // 数字
    QColor stringColor;       // 字符串
    QColor functionColor;     // 函数
    QColor commentColor;      // 注释
    QColor colorByName(const QString &name) const;// 🆕 根据颜色名字获取颜色

    // 静态方法：创建两种预设主题
    static Theme lightTheme();
    static Theme darkTheme();
};

#endif // THEME_H