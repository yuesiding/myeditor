#ifndef SYNTAXMANAGER_H
#define SYNTAXMANAGER_H

#include <QObject>
#include <QVector>
#include "syntaxdefinition.h"

// 语法管理器（单例）
// 启动时扫描 syntax/ 目录，加载所有 JSON
class SyntaxManager : public QObject
{
    Q_OBJECT

public:
    static SyntaxManager& instance();

    // 加载指定目录下所有 .json 文件
    void loadFromDirectory(const QString &dirPath);

    // 根据文件扩展名查找对应的语法定义
    // 找到返回 true，并把结果填入 def；找不到返回 false
    bool findByExtension(const QString &ext, SyntaxDefinition &def) const;

    // 获取所有已加载的语法定义
    QVector<SyntaxDefinition> allDefinitions() const { return m_definitions; }

private:
    SyntaxManager() {}
    SyntaxManager(const SyntaxManager&) = delete;
    SyntaxManager& operator=(const SyntaxManager&) = delete;

    QVector<SyntaxDefinition> m_definitions;
};

#endif // SYNTAXMANAGER_H