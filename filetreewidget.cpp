#include "filetreewidget.h"

#include <QTreeView>
#include <QFileSystemModel>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QAction>
#include <QDesktopServices>
#include <QUrl>
#include <QHeaderView>

FileTreeWidget::FileTreeWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void FileTreeWidget::setupUi()
{
    // ===== 顶部工具栏 =====
    m_openButton = new QPushButton(tr("📂 打开文件夹"), this);
    connect(m_openButton, &QPushButton::clicked,
            this, &FileTreeWidget::onOpenFolderClicked);

    m_folderLabel = new QLabel(tr("(未选择文件夹)"), this);
    m_folderLabel->setStyleSheet("QLabel { padding: 4px; color: #666; }");

    // ===== 文件树 =====
    m_model = new QFileSystemModel(this);
    m_model->setRootPath("");   // 空的话下面 setRootPath 再设

    m_treeView = new QTreeView(this);
    m_treeView->setModel(m_model);

    // 只显示第一列（文件名），隐藏其他（大小、日期等）
    m_treeView->hideColumn(1);
    m_treeView->hideColumn(2);
    m_treeView->hideColumn(3);
    m_treeView->header()->hide();

    // 双击打开
    connect(m_treeView, &QTreeView::doubleClicked,
            this, &FileTreeWidget::onTreeDoubleClicked);

    // 启用右键菜单
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_treeView, &QTreeView::customContextMenuRequested,
            this, &FileTreeWidget::onCustomContextMenu);

    // ===== 布局 =====
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->addWidget(m_openButton);
    layout->addWidget(m_folderLabel);
    layout->addWidget(m_treeView, 1);
    setLayout(layout);
        m_treeView->setStyleSheet(
        "QTreeView { "
        "  background-color: #F8F8F8; "
        "  border: none; "
        "  outline: 0; "
        "} "
        "QTreeView::item { padding: 4px; } "
        "QTreeView::item:hover { background-color: #E8E8E8; } "
        "QTreeView::item:selected { background-color: #C8E4FF; color: black; } "
    );
}

void FileTreeWidget::setRootPath(const QString &path)
{
    if (path.isEmpty()) return;

    QFileInfo info(path);
    if (!info.exists() || !info.isDir()) return;

    m_rootPath = path;

    // 设置模型的根 + 树视图的根
    m_model->setRootPath(path);
    m_treeView->setRootIndex(m_model->index(path));

    // 更新标签
    m_folderLabel->setText(info.fileName().isEmpty() ? path : info.fileName());
    m_folderLabel->setToolTip(path);
}

// ===== 打开文件夹按钮 =====
void FileTreeWidget::onOpenFolderClicked()
{
    QString path = QFileDialog::getExistingDirectory(
        this,
        tr("选择文件夹"),
        m_rootPath.isEmpty() ? QDir::homePath() : m_rootPath
    );

    if (!path.isEmpty()) {
        setRootPath(path);
    }
}

// ===== 双击处理 =====
void FileTreeWidget::onTreeDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    QString filePath = m_model->filePath(index);
    QFileInfo info(filePath);

    // 只对文件发出信号（文件夹的双击 QTreeView 会自动展开）
    if (info.isFile()) {
        emit fileDoubleClicked(filePath);
    }
}

// ===== 右键菜单 =====
void FileTreeWidget::onCustomContextMenu(const QPoint &pos)
{
    QModelIndex index = m_treeView->indexAt(pos);
    QString filePath = index.isValid() ? m_model->filePath(index) : m_rootPath;

    QMenu menu(this);

    QAction *openAction = menu.addAction(tr("📂 打开"));
    connect(openAction, &QAction::triggered, [this, filePath]() {
        QFileInfo info(filePath);
        if (info.isFile()) {
            emit fileDoubleClicked(filePath);
        }
    });

    menu.addSeparator();

    QAction *showInExplorer = menu.addAction(tr("在资源管理器中显示"));
    connect(showInExplorer, &QAction::triggered, [filePath]() {
        // 打开文件所在文件夹并选中该文件
        QFileInfo info(filePath);
        QString folder = info.isDir() ? filePath : info.absolutePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
    });

    QAction *refreshAction = menu.addAction(tr("🔄 刷新"));
    connect(refreshAction, &QAction::triggered, [this]() {
        // QFileSystemModel 自动监听文件变化，一般不用手动刷新
        // 但用户手动触发时，重新设置一次根目录
        if (!m_rootPath.isEmpty()) {
            setRootPath(m_rootPath);
        }
    });

    menu.exec(m_treeView->viewport()->mapToGlobal(pos));
}