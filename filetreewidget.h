#ifndef FILETREEWIDGET_H
#define FILETREEWIDGET_H

#include <QWidget>

class QTreeView;
class QFileSystemModel;
class QPushButton;
class QLabel;
class QModelIndex;

class FileTreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileTreeWidget(QWidget *parent = nullptr);

    // 设置根目录
    void setRootPath(const QString &path);

    // 获取当前根目录
    QString rootPath() const { return m_rootPath; }

signals:
    // 用户双击文件时发出
    void fileDoubleClicked(const QString &filePath);

private slots:
    void onOpenFolderClicked();
    void onTreeDoubleClicked(const QModelIndex &index);
    void onCustomContextMenu(const QPoint &pos);

private:
    void setupUi();

    QLabel *m_folderLabel;         // 显示当前文件夹名
    QPushButton *m_openButton;     // "打开文件夹"按钮
    QTreeView *m_treeView;         // 树形视图
    QFileSystemModel *m_model;     // 文件系统模型

    QString m_rootPath;
};

#endif // FILETREEWIDGET_H