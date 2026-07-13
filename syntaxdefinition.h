#ifndef SYNTAXDEFINITION_H
#define SYNTAXDEFINITION_H

#include <QString>
#include <QStringList>
#include <QVector>

struct CustomPattern {
    QString name;      //规则名
    QString pattern;   //正则表达式
    QString color;     //颜色名
};

class SyntaxDefinition
{
public:
    SyntaxDefinition();
    //从JSON文件
    bool loadFromFile(const QString &filePath);
    //属性访问
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