#include "mainwindow.h"
#include "syntaxmanager.h"

#include <QApplication>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 加载语法定义
    QString appDir = QCoreApplication::applicationDirPath();
    QString syntaxDir = appDir + "/syntax";
    SyntaxManager::instance().loadFromDirectory(syntaxDir);

    MainWindow window;
    window.show();

    // 🆕 处理命令行参数
    // argv[0] 是程序自己的路径，argv[1] 开始才是用户传的参数
    for (int i = 1; i < argc; ++i) {
        QString filePath = QString::fromLocal8Bit(argv[i]);
        // 支持相对路径
        QFileInfo fi(filePath);
        if (!fi.isAbsolute()) {
            filePath = QDir::current().absoluteFilePath(filePath);
        }
        window.openFileByPath(filePath);
    }

    return app.exec();
}