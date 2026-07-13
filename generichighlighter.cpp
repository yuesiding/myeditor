#include "generichighlighter.h"
#include "thememanager.h"

GenericHighlighter::GenericHighlighter(const SyntaxDefinition &def, QTextDocument *parent):Highlighter(parent), m_definition(def){
    setupRules();
}

void GenericHighlighter::setupRules(){
    const Theme &theme = ThemeManager::instance().currentTheme();
    HighlightRule rule;

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(theme.keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);

    for (const QString &kw : m_definition.keywords()) {
        rule.pattern = QRegularExpression("\\b" + kw + "\\b");
        rule.format = keywordFormat;
        m_rules.append(rule);
    }
    QTextCharFormat typeFormat;
    typeFormat.setForeground(theme.qtClassColor);
    typeFormat.setFontWeight(QFont::Bold);

    for (const QString &t : m_definition.types()) {
        rule.pattern = QRegularExpression("\\b" + t + "\\b");
        rule.format = typeFormat;
        m_rules.append(rule);
    }

    //自定义
    for (const CustomPattern &cp : m_definition.customPatterns()) {
        QTextCharFormat fmt;
        fmt.setForeground(theme.colorByName(cp.color));
        fmt.setFontWeight(QFont::Bold);
        rule.pattern = QRegularExpression(cp.pattern);
        rule.format = fmt;
        m_rules.append(rule);
    }
    //预处理
    if (!m_definition.preprocessorPattern().isEmpty()) {
        QTextCharFormat preprocessorFormat;
        preprocessorFormat.setForeground(theme.preprocessorColor);
        rule.pattern = QRegularExpression(m_definition.preprocessorPattern());
        rule.format = preprocessorFormat;
        m_rules.append(rule);
    }
    //数字
    if (!m_definition.numberPattern().isEmpty()) {
        QTextCharFormat numberFormat;
        numberFormat.setForeground(theme.numberColor);
        rule.pattern = QRegularExpression(m_definition.numberPattern());
        rule.format = numberFormat;
        m_rules.append(rule);
    }
    //字符串
    QTextCharFormat stringFormat;
    stringFormat.setForeground(theme.stringColor);
    for (const QString &pattern : m_definition.stringPatterns()) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = stringFormat;
        m_rules.append(rule);
    }
    //函数
    if (!m_definition.functionPattern().isEmpty()) {
        QTextCharFormat functionFormat;
        functionFormat.setForeground(theme.functionColor);
        rule.pattern = QRegularExpression(m_definition.functionPattern());
        rule.format = functionFormat;
        m_rules.append(rule);
    }
    //单行注释
    if (!m_definition.singleLineComment().isEmpty()) {
        QTextCharFormat singleLineCommentFormat;
        singleLineCommentFormat.setForeground(theme.commentColor);
        singleLineCommentFormat.setFontItalic(true);
        QString commentStart = QRegularExpression::escape(m_definition.singleLineComment());
        rule.pattern = QRegularExpression(commentStart + "[^\n]*");
        rule.format = singleLineCommentFormat;
        m_rules.append(rule);
    }
    //多行注释
    if (!m_definition.multiLineCommentStart().isEmpty()&&!m_definition.multiLineCommentEnd().isEmpty()) {
        m_multiLineCommentFormat.setForeground(theme.commentColor);
        m_multiLineCommentFormat.setFontItalic(true);
        m_commentStartExpression = QRegularExpression(QRegularExpression::escape(m_definition.multiLineCommentStart()));
        m_commentEndExpression = QRegularExpression(QRegularExpression::escape(m_definition.multiLineCommentEnd()));
    }
}