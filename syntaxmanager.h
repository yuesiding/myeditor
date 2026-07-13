#ifndef SYNTAXMANAGER_H
#define SYNTAXMANAGER_H

#include <QObject>
#include <QVector>
#include "syntaxdefinition.h"

//启动时扫描syntax/目录，加载所有JSON
class SyntaxManager : public QObject
{
    Q_OBJECT

public:
    static SyntaxManager& instance();
    void loadFromDirectory(const QString &dirPath);// 加载指定目录下所有 .json 文件
    bool findByExtension(const QString &ext, SyntaxDefinition &def) const; // 根据扩展名查找对应的语法定义
    QVector<SyntaxDefinition> allDefinitions() const { return m_definitions; }// 获取所有已加载的语法定义
private:
    SyntaxManager(){}
    SyntaxManager(const SyntaxManager&)=delete;
    SyntaxManager& operator=(const SyntaxManager&)=delete;
    QVector<SyntaxDefinition> m_definitions;
};

#endif // SYNTAXMANAGER_H