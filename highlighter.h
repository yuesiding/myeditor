#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>

class Highlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit Highlighter(QTextDocument *parent = nullptr);

    // 🆕 子类实现：根据当前主题设置颜色
    virtual void setupRules() = 0;

protected:
    void highlightBlock(const QString &text) override;

    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightRule> m_rules;

    QRegularExpression m_commentStartExpression;
    QRegularExpression m_commentEndExpression;
    QTextCharFormat m_multiLineCommentFormat;

private slots:
    // 🆕 主题变化时重新设置规则
    void onThemeChanged();
};

#endif // HIGHLIGHTER_H