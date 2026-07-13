#include "syntaxmanager.h"
#include <QDir>
#include <QDirIterator>
#include <QDebug>

SyntaxManager& SyntaxManager::instance()
{
    static SyntaxManager instance;
    return instance;
}

void SyntaxManager::loadFromDirectory(const QString &dirPath)
{
    m_definitions.clear();

    QDir dir(dirPath);
    if (!dir.exists()) {
        qWarning()<<"语法目录不存在:"<<dirPath;
        return;
    }
    //遍历所有.json
    QStringList filters;
    filters << "*.json";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    for (const QFileInfo &fi : files) {
        SyntaxDefinition def;
        if (def.loadFromFile(fi.absoluteFilePath())) {
            m_definitions.append(def);
        }
    }
    qDebug() << "语法管理器加载完成，共" << m_definitions.size() << "种语言";
}
bool SyntaxManager::findByExtension(const QString &ext, SyntaxDefinition &def) const
{
    for (const SyntaxDefinition &d:m_definitions) {
        if (d.matchesExtension(ext)) {
            def=d;
            return true;
        }
    }
    return false;
}