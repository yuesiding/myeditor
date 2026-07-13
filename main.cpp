#include "mainwindow.h"
#include "syntaxmanager.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 加载语法定义
    QString appDir = QCoreApplication::applicationDirPath();
    QString syntaxDir = appDir + "/syntax";
    SyntaxManager::instance().loadFromDirectory(syntaxDir);

    MainWindow window;
    window.show();

    return app.exec();
}