#include "highlighter.h"
#include "thememanager.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // 监听主题变化
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &Highlighter::onThemeChanged);
}

void Highlighter::onThemeChanged()
{
    // 清空旧规则
    m_rules.clear();
    // 让子类重新设置
    setupRules();
    // 重新高亮所有内容
    rehighlight();
}

void Highlighter::highlightBlock(const QString &text)
{
    for (const HighlightRule &rule : m_rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    if (m_commentStartExpression.pattern().isEmpty()) {
        return;
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf(m_commentStartExpression);
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch;
        int endIndex = text.indexOf(m_commentEndExpression, startIndex, &endMatch);

        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }

        setFormat(startIndex, commentLength, m_multiLineCommentFormat);

        startIndex = text.indexOf(m_commentStartExpression,
                                  startIndex + commentLength);
    }
}