#include "syntaxdefinition.h"
#include <QSettings>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

SyntaxDefinition::SyntaxDefinition(){}
bool SyntaxDefinition::loadFromFile(const QString &filePath)
{
    //打开文件
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开语法文件:" << filePath;
        return false;
    }
    //读取内容
    QByteArray data=file.readAll();
    file.close();
    // 解析 JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误:" << parseError.errorString()
                   << "文件:" << filePath;
        return false;
    }
    if (!doc.isObject()){
        qWarning() << "JSON 根不是对象:" << filePath;
        return false;
    }
    QJsonObject root=doc.object();

    m_name = root.value("name").toString();
    QJsonArray extArray = root.value("extensions").toArray();
    m_extensions.clear();
    for (const QJsonValue &v : extArray) {
        m_extensions.append(v.toString().toLower());
    }
    //关键字
    QJsonArray kwArray=root.value("keywords").toArray();
    m_keywords.clear();
    for (const QJsonValue &v : kwArray){
        m_keywords.append(v.toString());
    }
    //类型列表
    QJsonArray typeArray = root.value("types").toArray();
    m_types.clear();
    for (const QJsonValue &v : typeArray) {
        m_types.append(v.toString());
    }
    //注释
    m_singleLineComment = root.value("singleLineComment").toString();
    m_multiLineCommentStart = root.value("multiLineCommentStart").toString();
    m_multiLineCommentEnd = root.value("multiLineCommentEnd").toString();
    //字符串
    QJsonArray strArray = root.value("stringPatterns").toArray();
    m_stringPatterns.clear();
    for (const QJsonValue &v : strArray) {
        m_stringPatterns.append(v.toString());
    }
    //单独的
    m_numberPattern = root.value("numberPattern").toString();
    m_functionPattern = root.value("functionPattern").toString();
    m_preprocessorPattern = root.value("preprocessorPattern").toString();
    //自定义规则
    QJsonArray customArray = root.value("customPatterns").toArray();
    m_customPatterns.clear();
    for (const QJsonValue &v : customArray) {
        QJsonObject obj = v.toObject();
        CustomPattern cp;
        cp.name = obj.value("name").toString();
        cp.pattern = obj.value("pattern").toString();
        cp.color = obj.value("color").toString();
        m_customPatterns.append(cp);
    }
    qDebug() << "成功加载语法定义:" << m_name
             << "扩展名:" << m_extensions
             << "关键字数量:" << m_keywords.size();
    return true;
}
bool SyntaxDefinition::matchesExtension(const QString &ext) const
{
    return m_extensions.contains(ext.toLower());
}