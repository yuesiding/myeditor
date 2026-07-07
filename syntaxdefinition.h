#ifndef SYNTAXDEFINITION_H
#define SYNTAXDEFINITION_H

#include <QString>
#include <QStringList>
#include <QVector>

// 自定义匹配规则（比如 Qt 类、Python 装饰器等）
struct CustomPattern {
    QString name;      // 规则名（便于识别）
    QString pattern;   // 正则表达式
    QString color;     // 对应主题里的颜色名
};

// 一个语法定义 = 一个 JSON 文件的内容
class SyntaxDefinition
{
public:
    SyntaxDefinition();

    // 从 JSON 文件加载
    bool loadFromFile(const QString &filePath);

    // 属性访问
    QString name() const { return m_name; }
    QStringList extensions() const { return m_extensions; }
    QStringList keywords() const { return m_keywords; }
    QStringList types() const { return m_types; }

    QString singleLineComment() const { return m_singleLineComment; }
    QString multiLineCommentStart() const { return m_multiLineCommentStart; }
    QString multiLineCommentEnd() const { return m_multiLineCommentEnd; }

    QStringList stringPatterns() const { return m_stringPatterns; }
    QString numberPattern() const { return m_numberPattern; }
    QString functionPattern() const { return m_functionPattern; }
    QString preprocessorPattern() const { return m_preprocessorPattern; }

    QVector<CustomPattern> customPatterns() const { return m_customPatterns; }

    // 判断文件扩展名是否匹配这个语言
    bool matchesExtension(const QString &ext) const;

private:
    QString m_name;
    QStringList m_extensions;
    QStringList m_keywords;
    QStringList m_types;

    QString m_singleLineComment;
    QString m_multiLineCommentStart;
    QString m_multiLineCommentEnd;

    QStringList m_stringPatterns;
    QString m_numberPattern;
    QString m_functionPattern;
    QString m_preprocessorPattern;

    QVector<CustomPattern> m_customPatterns;
};

#endif // SYNTAXDEFINITION_H