#include "mainwindow.h"
#include "editorwidget.h"
#include "findreplacedialog.h"
#include "thememanager.h"
#include "preferencesdialog.h"
#include "filetreewidget.h"
#include <QDockWidget>
#include <QLabel>
#include <QTabWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QCloseEvent>
#include <QPlainTextEdit>   
#include <QSettings>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QToolBar>
#include <QStyle>
#include <QIcon>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_findReplaceDialog(nullptr)
{
    m_cursorPositionLabel = new QLabel(tr("行:1 列:1"), this);
    m_encodingLabel = new QLabel(tr("UTF-8"), this);
    m_charCountLabel = new QLabel(tr("0 字符"), this);
    m_foldCountLabel = new QLabel(tr(""), this);

    m_cursorPositionLabel->setMinimumWidth(120);
    m_encodingLabel->setMinimumWidth(60);
    m_charCountLabel->setMinimumWidth(100);
    m_foldCountLabel->setMinimumWidth(100);

    statusBar()->addPermanentWidget(m_cursorPositionLabel);
    statusBar()->addPermanentWidget(m_encodingLabel);
    statusBar()->addPermanentWidget(m_charCountLabel);
    statusBar()->addPermanentWidget(m_foldCountLabel); 
    
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::closeTab);
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::onCurrentTabChanged);

    setCentralWidget(m_tabWidget);

    createMenus();
    createToolBars(); 
    // 🆕 创建文件树 dock
    m_fileTreeWidget = new FileTreeWidget(this);
    m_fileTreeDock = new QDockWidget(tr("文件浏览器"), this);
    m_fileTreeDock->setObjectName("fileTreeDock");   // 用于会话恢复
    m_fileTreeDock->setWidget(m_fileTreeWidget);
    m_fileTreeDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_fileTreeDock);

    // 连接：文件树双击 → 打开文件
    connect(m_fileTreeWidget, &FileTreeWidget::fileDoubleClicked,
            this, &MainWindow::onFileTreeDoubleClicked);

    // 从 QSettings 恢复上次的文件夹
    QSettings settings("MyCompany", "CodeEditor");
    QString lastFolder = settings.value("fileTree/rootPath").toString();
    if (!lastFolder.isEmpty()) {
        m_fileTreeWidget->setRootPath(lastFolder);
    }

    createEditor();   // ← 这里会触发 onCurrentTabChanged，  必须让状态栏先初始化好

    resize(900, 600);
    setWindowTitle(tr("CodeEditor"));
    loadRecentFiles(); //  加载最近文件历史
    setAcceptDrops(true);
        
    restoreSession();//  恢复上次会话
    updateRecentFilesMenu();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createMenus()
{
    // ===== 文件菜单 =====
    QMenu *fileMenu = menuBar()->addMenu(tr("文件(&F)"));

    QAction *newAction = new QAction(tr("新建(&N)"), this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAction);

    QAction *openAction = new QAction(tr("打开(&O)..."), this);
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    fileMenu->addAction(openAction);

    QAction *saveAction = new QAction(tr("保存(&S)"), this);
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    fileMenu->addAction(saveAction);

    QAction *saveAsAction = new QAction(tr("另存为(&A)..."), this);
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAsFile);
    fileMenu->addAction(saveAsAction);
        //  最近打开子菜单
    fileMenu->addSeparator();
    m_recentFilesMenu = fileMenu->addMenu(tr("最近打开(&R)"));
    updateRecentFilesMenu();   // 初始化菜单内容
    fileMenu->addSeparator();

    QAction *exitAction = new QAction(tr("退出(&X)"), this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);

        // ===== 🆕 视图菜单 =====
    QMenu *viewMenu = menuBar()->addMenu(tr("视图(&V)"));

    QMenu *themeMenu = viewMenu->addMenu(tr("主题(&T)"));
        // 🆕 字体缩放菜单项
    viewMenu->addSeparator();
    QAction *zoomInAction = new QAction(tr("放大字体"), this);
    zoomInAction->setShortcut(QKeySequence::ZoomIn);   // Ctrl++
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);
    viewMenu->addAction(zoomInAction);

    QAction *zoomOutAction = new QAction(tr("缩小字体"), this);
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);   // Ctrl+-
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);
    viewMenu->addAction(zoomOutAction);

    QAction *resetZoomAction = new QAction(tr("重置字体大小"), this);
    resetZoomAction->setShortcut(tr("Ctrl+0"));
    connect(resetZoomAction, &QAction::triggered, this, &MainWindow::resetZoom);
    viewMenu->addAction(resetZoomAction);

    QAction *lightThemeAction = new QAction(tr("亮色主题"), this);
    connect(lightThemeAction, &QAction::triggered,
            this, &MainWindow::switchToLightTheme);
    themeMenu->addAction(lightThemeAction);

    QAction *darkThemeAction = new QAction(tr("暗色主题"), this);
    connect(darkThemeAction, &QAction::triggered,
            this, &MainWindow::switchToDarkTheme);
    themeMenu->addAction(darkThemeAction);

    // ===== 🆕 编辑菜单 =====
    QMenu *editMenu = menuBar()->addMenu(tr("编辑(&E)"));

    QAction *findAction = new QAction(tr("查找和替换(&F)..."), this);
    findAction->setShortcut(QKeySequence::Find);   // Ctrl+F
    connect(findAction, &QAction::triggered, this, &MainWindow::showFindReplaceDialog);
    editMenu->addAction(findAction);
        // 🆕 显示/隐藏文件树
    m_toggleFileTreeAction = new QAction(tr("显示文件浏览器(&E)"), this);
    m_toggleFileTreeAction->setCheckable(true);
    m_toggleFileTreeAction->setChecked(true);
    m_toggleFileTreeAction->setShortcut(tr("Ctrl+B"));
    connect(m_toggleFileTreeAction, &QAction::triggered,
            this, &MainWindow::toggleFileTree);
    viewMenu->addAction(m_toggleFileTreeAction);

    // 🆕 打开文件夹菜单项
    QAction *openFolderAction = new QAction(tr("打开文件夹(&D)..."), this);
    openFolderAction->setShortcut(tr("Ctrl+K, Ctrl+O"));
    connect(openFolderAction, &QAction::triggered,
            this, &MainWindow::openFolderInTree);
    viewMenu->addAction(openFolderAction);

    viewMenu->addSeparator();

            // 🆕 折叠菜单
    viewMenu->addSeparator();
    QAction *foldAllAction = new QAction(tr("折叠所有"), this);
    foldAllAction->setShortcut(tr("Ctrl+K, Ctrl+0"));
    connect(foldAllAction, &QAction::triggered, this, &MainWindow::foldAll);
    viewMenu->addAction(foldAllAction);

    QAction *unfoldAllAction = new QAction(tr("展开所有"), this);
    unfoldAllAction->setShortcut(tr("Ctrl+K, Ctrl+J"));
    connect(unfoldAllAction, &QAction::triggered, this, &MainWindow::unfoldAll);
    viewMenu->addAction(unfoldAllAction);

    viewMenu->addSeparator();
    QAction *prefAction = new QAction(tr("首选项(&P)..."), this);
    prefAction->setShortcut(tr("Ctrl+,"));
    connect(prefAction, &QAction::triggered, this, &MainWindow::showPreferences);
    viewMenu->addAction(prefAction);
}

EditorWidget *MainWindow::createEditor()
{
    EditorWidget *editor = new EditorWidget(this);
    int index = m_tabWidget->addTab(editor, editor->userFriendlyName());
    m_tabWidget->setCurrentIndex(index);

    connect(editor, &EditorWidget::fileInfoChanged,
            this, &MainWindow::updateTabTitle);

    return editor;
}

EditorWidget *MainWindow::currentEditor() const
{
    return qobject_cast<EditorWidget *>(m_tabWidget->currentWidget());
}

void MainWindow::newFile()
{
    createEditor();
}

void MainWindow::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"));
    openFileByPath(fileName);
}
void MainWindow::saveFile()
{
    EditorWidget *editor = currentEditor();
    if (editor) {
        editor->save();
    }
}

void MainWindow::saveAsFile()
{
    EditorWidget *editor = currentEditor();
    if (!editor) return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"));
    if (fileName.isEmpty()) return;

    editor->saveAs(fileName);
}

void MainWindow::closeTab(int index)
{
    EditorWidget *editor = qobject_cast<EditorWidget *>(m_tabWidget->widget(index));
    if (!editor) return;

    if (!editor->maybeSave()) {
        return;
    }

    m_tabWidget->removeTab(index);
    editor->deleteLater();

    if (m_tabWidget->count() == 0) {
        createEditor();
    }
}

void MainWindow::updateTabTitle()
{
    EditorWidget *editor = qobject_cast<EditorWidget *>(sender());
    if (!editor) return;

    int index = m_tabWidget->indexOf(editor);
    if (index < 0) return;

    QString title = editor->userFriendlyName();
    if (editor->isModified()) {
        title += " *";
    }

    m_tabWidget->setTabText(index, title);
}

void MainWindow::onCurrentTabChanged(int index)
{
    Q_UNUSED(index);
    EditorWidget *editor = currentEditor();
    if (editor) {
        QString name = editor->currentFile().isEmpty() ?
                       tr("未命名") : editor->currentFile();
        statusBar()->showMessage(name);   // 左侧：文件路径

        // 🆕 连接光标和内容变化信号（先断开避免重复连接）
        disconnect(editor, &QPlainTextEdit::cursorPositionChanged,
                   this, &MainWindow::updateStatusBarInfo);
        disconnect(editor, &QPlainTextEdit::textChanged,
                   this, &MainWindow::updateStatusBarInfo);

        connect(editor, &QPlainTextEdit::cursorPositionChanged,
                this, &MainWindow::updateStatusBarInfo);
        connect(editor, &QPlainTextEdit::textChanged,
                this, &MainWindow::updateStatusBarInfo);
            // 🆕 连接折叠数量变化
    disconnect(editor, &EditorWidget::foldCountChanged,
               this, &MainWindow::updateFoldCount);
    connect(editor, &EditorWidget::foldCountChanged,
            this, &MainWindow::updateFoldCount);

    // 立即更新一次
    updateFoldCount(editor->foldedCount());
        // 立即更新一次
        updateStatusBarInfo();

        if (m_findReplaceDialog) {
            m_findReplaceDialog->setEditor(editor);
        }
    } else {
        statusBar()->clearMessage();
    }
}

// ===== 🆕 显示查找替换对话框 =====
void MainWindow::showFindReplaceDialog()
{
    EditorWidget *editor = currentEditor();
    if (!editor) return;

    if (!m_findReplaceDialog) {
        m_findReplaceDialog = new FindReplaceDialog(editor, this);
    } else {
        m_findReplaceDialog->setEditor(editor);
    }

    m_findReplaceDialog->show();
    m_findReplaceDialog->raise();     // 提到最前
    m_findReplaceDialog->activateWindow();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // 先询问未保存的文件
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        EditorWidget *editor = qobject_cast<EditorWidget *>(m_tabWidget->widget(i));
        if (editor && !editor->maybeSave()) {
            event->ignore();
            return;
        }
    }
    saveSession(); // 保存会话

    event->accept();
}

// ===== 🆕 主题切换 =====
void MainWindow::switchToLightTheme()
{
    ThemeManager::instance().setLightTheme();
}

void MainWindow::switchToDarkTheme()
{
    ThemeManager::instance().setDarkTheme();
}

// ===== 🆕 更新状态栏信息 =====
void MainWindow::updateStatusBarInfo()
{
    if (!m_cursorPositionLabel || !m_encodingLabel || !m_charCountLabel) {
        return;
    }
    EditorWidget *editor = currentEditor();
    if (!editor) return;

    // 获取光标位置
    QTextCursor cursor = editor->textCursor();
    int line = cursor.blockNumber() + 1;      // 从 1 开始
    int column = cursor.columnNumber() + 1;

    // 更新行列号
    m_cursorPositionLabel->setText(tr("行:%1 列:%2").arg(line).arg(column));

    // 更新字符数
    int charCount = editor->document()->characterCount() - 1;  // -1 是因为末尾有个隐藏字符
    m_charCountLabel->setText(tr("%1 字符").arg(charCount));
}

// ============================================================
// 🆕 最近打开的文件
// ============================================================

// 从 QSettings 加载历史
void MainWindow::loadRecentFiles()
{
    QSettings settings("MyCompany", "CodeEditor");
    m_recentFiles = settings.value("recentFiles").toStringList();

    // 清理不存在的文件
    QStringList existing;
    for (const QString &path : m_recentFiles) {
        if (QFileInfo::exists(path)) {
            existing.append(path);
        }
    }
    m_recentFiles = existing;
}

// 保存到 QSettings
void MainWindow::saveRecentFiles()
{
    QSettings settings("MyCompany", "CodeEditor");
    settings.setValue("recentFiles", m_recentFiles);
}

// 添加一个文件到历史
void MainWindow::addRecentFile(const QString &filePath)
{
    // 如果已存在，先移除（要放到最前面）
    m_recentFiles.removeAll(filePath);

    // 加到最前面
    m_recentFiles.prepend(filePath);

    // 限制最多 MaxRecentFiles 个
    while (m_recentFiles.size() > MaxRecentFiles) {
        m_recentFiles.removeLast();
    }

    // 保存 + 刷新菜单
    saveRecentFiles();
    updateRecentFilesMenu();
}

// 刷新菜单显示
void MainWindow::updateRecentFilesMenu()
{
    if (!m_recentFilesMenu) return;

    m_recentFilesMenu->clear();   // 清空现有菜单项

    if (m_recentFiles.isEmpty()) {
        // 空历史时，显示一个禁用的"（无）"
        QAction *emptyAction = m_recentFilesMenu->addAction(tr("(无)"));
        emptyAction->setEnabled(false);
        return;
    }

    // 添加每个文件
    for (int i = 0; i < m_recentFiles.size(); ++i) {
        QString filePath = m_recentFiles.at(i);
        QString fileName = QFileInfo(filePath).fileName();

        // 菜单项文字：序号 + 文件名
        QString itemText = tr("%1  %2").arg(i + 1).arg(filePath);

        QAction *action = m_recentFilesMenu->addAction(itemText);
        // 把完整路径存到 action 的 data 里
        action->setData(filePath);
        connect(action, &QAction::triggered, this, &MainWindow::openRecentFile);
    }

    // 分隔线 + 清空按钮
    m_recentFilesMenu->addSeparator();
    QAction *clearAction = m_recentFilesMenu->addAction(tr("清空历史记录"));
    connect(clearAction, &QAction::triggered, this, &MainWindow::clearRecentFiles);
}

// 点击某个最近文件时触发
void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) return;

    QString filePath = action->data().toString();

    // 如果文件不存在，从历史里移除
    if (!QFileInfo::exists(filePath)) {
        QMessageBox::warning(this, tr("警告"),
                             tr("文件不存在：%1").arg(filePath));
        m_recentFiles.removeAll(filePath);
        saveRecentFiles();
        updateRecentFilesMenu();
        return;
    }

    openFileByPath(filePath);
}

// 清空历史
void MainWindow::clearRecentFiles()
{
    m_recentFiles.clear();
    saveRecentFiles();
    updateRecentFilesMenu();
}

// ============================================================
// 🆕 打开指定路径的文件（供外部调用）
// ============================================================
void MainWindow::openFileByPath(const QString &filePath)
{
    if (filePath.isEmpty()) return;

    // 检查文件是否存在
    if (!QFileInfo::exists(filePath)) {
        QMessageBox::warning(this, tr("警告"),
                             tr("文件不存在：%1").arg(filePath));
        return;
    }

    // 检查是否已打开
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        EditorWidget *editor = qobject_cast<EditorWidget *>(m_tabWidget->widget(i));
        if (editor && editor->currentFile() == filePath) {
            m_tabWidget->setCurrentIndex(i);
            addRecentFile(filePath);
            return;
        }
    }

    // 决定用当前标签还是新建标签
    EditorWidget *editor = currentEditor();
    if (!editor || !editor->currentFile().isEmpty() || editor->isModified()) {
        editor = createEditor();
    }

    // 加载
    if (editor->loadFile(filePath)) {
        addRecentFile(filePath);
    } else {
        int idx = m_tabWidget->indexOf(editor);
        if (idx >= 0) {
            m_tabWidget->removeTab(idx);
            editor->deleteLater();
        }
    }
}

// ============================================================
// 🆕 拖放事件处理
// ============================================================

// 用户拖东西进来时触发（判断要不要接受）
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    // 只接受"文件"类型的拖放
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

// 用户放下时触发（真正处理）
void MainWindow::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (!mimeData->hasUrls()) return;

    // 遍历所有拖进来的文件
    QList<QUrl> urls = mimeData->urls();
    for (const QUrl &url : urls) {
        QString filePath = url.toLocalFile();
        if (!filePath.isEmpty()) {
            openFileByPath(filePath);
        }
    }

    event->acceptProposedAction();
}

// ============================================================
// 🆕 字体缩放
// ============================================================
void MainWindow::zoomIn()
{
    EditorWidget *editor = currentEditor();
    if (editor) editor->zoomIn();
}

void MainWindow::zoomOut()
{
    EditorWidget *editor = currentEditor();
    if (editor) editor->zoomOut();
}

void MainWindow::resetZoom()
{
    EditorWidget *editor = currentEditor();
    if (editor) editor->resetZoom();
}

// ============================================================
// 🆕 创建工具栏
// ============================================================
void MainWindow::createToolBars()
{
    // 拿到 Qt 的标准图标提供者
    QStyle *style = this->style();

    // ===== 文件工具栏 =====
    m_fileToolBar = addToolBar(tr("文件"));
    m_fileToolBar->setObjectName("fileToolBar");   // 用于保存状态

    QAction *newAct = m_fileToolBar->addAction(
        style->standardIcon(QStyle::SP_FileIcon),
        tr("新建"));
    newAct->setShortcut(QKeySequence::New);
    newAct->setToolTip(tr("新建文件 (Ctrl+N)"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    QAction *openAct = m_fileToolBar->addAction(
        style->standardIcon(QStyle::SP_DialogOpenButton),
        tr("打开"));
    openAct->setShortcut(QKeySequence::Open);
    openAct->setToolTip(tr("打开文件 (Ctrl+O)"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

    QAction *saveAct = m_fileToolBar->addAction(
        style->standardIcon(QStyle::SP_DialogSaveButton),
        tr("保存"));
    saveAct->setShortcut(QKeySequence::Save);
    saveAct->setToolTip(tr("保存文件 (Ctrl+S)"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);

    // ===== 编辑工具栏 =====
    m_editToolBar = addToolBar(tr("编辑"));
    m_editToolBar->setObjectName("editToolBar");

    QAction *undoAct = m_editToolBar->addAction(
        style->standardIcon(QStyle::SP_ArrowBack),
        tr("撤销"));
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setToolTip(tr("撤销 (Ctrl+Z)"));
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);

    QAction *redoAct = m_editToolBar->addAction(
        style->standardIcon(QStyle::SP_ArrowForward),
        tr("重做"));
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setToolTip(tr("重做 (Ctrl+Y)"));
    connect(redoAct, &QAction::triggered, this, &MainWindow::redo);

    m_editToolBar->addSeparator();

    QAction *findAct = m_editToolBar->addAction(
        style->standardIcon(QStyle::SP_FileDialogContentsView),
        tr("查找"));
    findAct->setShortcut(QKeySequence::Find);
    findAct->setToolTip(tr("查找和替换 (Ctrl+F)"));
    connect(findAct, &QAction::triggered, this, &MainWindow::showFindReplaceDialog);
}

// ===== 🆕 撤销重做槽函数 =====
void MainWindow::undo()
{
    EditorWidget *editor = currentEditor();
    if (editor) editor->undo();
}

void MainWindow::redo()
{
    EditorWidget *editor = currentEditor();
    if (editor) editor->redo();
}

// ============================================================
// 🆕 会话保存与恢复
// ============================================================

void MainWindow::saveSession()
{
    QSettings settings("MyCompany", "CodeEditor");

    // ===== 保存窗口状态 =====
    settings.setValue("geometry", saveGeometry());       // 窗口大小和位置
    settings.setValue("windowState", saveState());       // 工具栏位置等

    // ===== 保存打开的文件 =====
    QStringList openFiles;
    QList<int> cursorPositions;

    for (int i = 0; i < m_tabWidget->count(); ++i) {
        EditorWidget *editor = qobject_cast<EditorWidget *>(m_tabWidget->widget(i));
        if (editor && !editor->currentFile().isEmpty()) {
            openFiles.append(editor->currentFile());
            cursorPositions.append(editor->cursorPosition());
        }
    }

    settings.setValue("session/openFiles", openFiles);

    // QSettings 不直接支持 QList<int>，转成 QVariantList
    QVariantList cursorList;
    for (int pos : cursorPositions) {
        cursorList.append(pos);
    }
    settings.setValue("session/cursorPositions", cursorList);

    // 当前激活的标签索引
    settings.setValue("session/currentIndex", m_tabWidget->currentIndex());
        
    if (m_fileTreeWidget) {// 保存文件树的当前文件夹
        settings.setValue("fileTree/rootPath", m_fileTreeWidget->rootPath());
    }
}

void MainWindow::restoreSession()
{
    QSettings settings("MyCompany", "CodeEditor");

    // ===== 恢复窗口大小和位置 =====
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }

    // ===== 恢复工具栏位置等 =====
    QByteArray windowState = settings.value("windowState").toByteArray();
    if (!windowState.isEmpty()) {
        restoreState(windowState);
    }

    // ===== 恢复打开的文件 =====
    QStringList openFiles = settings.value("session/openFiles").toStringList();
    QVariantList cursorList = settings.value("session/cursorPositions").toList();

    if (openFiles.isEmpty()) {
        return;   // 没有会话可恢复
    }

    // 移除默认的空白标签（因为构造函数里 createEditor 建了一个）
    while (m_tabWidget->count() > 0) {
        QWidget *w = m_tabWidget->widget(0);
        m_tabWidget->removeTab(0);
        w->deleteLater();
    }

    // 依次打开每个文件
    for (int i = 0; i < openFiles.size(); ++i) {
        QString filePath = openFiles.at(i);

        // 检查文件是否还存在
        if (!QFileInfo::exists(filePath)) {
            continue;   // 跳过已被删除的文件
        }

        EditorWidget *editor = createEditor();
        if (editor->loadFile(filePath)) {
            // 恢复光标位置
            if (i < cursorList.size()) {
                int pos = cursorList.at(i).toInt();
                editor->setCursorPosition(pos);
            }
        }
    }

    // 如果所有文件都不存在，恢复一个空白标签
    if (m_tabWidget->count() == 0) {
        createEditor();
    }

    // 恢复当前激活的标签
    int currentIndex = settings.value("session/currentIndex", 0).toInt();
    if (currentIndex >= 0 && currentIndex < m_tabWidget->count()) {
        m_tabWidget->setCurrentIndex(currentIndex);
    }
}

// ============================================================
// 🆕 首选项
// ============================================================
void MainWindow::showPreferences()
{
    PreferencesDialog dialog(this);
    connect(&dialog, &PreferencesDialog::settingsApplied,
            this, &MainWindow::applySettingsToAllEditors);
    dialog.exec();
}

void MainWindow::applySettingsToAllEditors()
{
    // 遍历所有标签，让每个编辑器重新加载设置
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        EditorWidget *editor = qobject_cast<EditorWidget *>(m_tabWidget->widget(i));
        if (editor) {
            editor->applySettings();
        }
    }
}

// ============================================================
// 🆕 文件树相关
// ============================================================

// 切换显示/隐藏
void MainWindow::toggleFileTree()
{
    if (m_fileTreeDock->isVisible()) {
        m_fileTreeDock->hide();
        m_toggleFileTreeAction->setChecked(false);
    } else {
        m_fileTreeDock->show();
        m_toggleFileTreeAction->setChecked(true);
    }
}

// 打开一个文件夹到文件树
void MainWindow::openFolderInTree()
{
    QString path = QFileDialog::getExistingDirectory(
        this,
        tr("选择要浏览的文件夹"),
        QDir::homePath()
    );

    if (!path.isEmpty()) {
        m_fileTreeWidget->setRootPath(path);
        // 确保文件树是可见的
        if (!m_fileTreeDock->isVisible()) {
            m_fileTreeDock->show();
            m_toggleFileTreeAction->setChecked(true);
        }
    }
}

// 文件树双击 → 打开文件
void MainWindow::onFileTreeDoubleClicked(const QString &filePath)
{
    openFileByPath(filePath);
}

// ============================================================
// 🆕 折叠相关
// ============================================================
void MainWindow::updateFoldCount(int count)
{
    if (count > 0) {
        m_foldCountLabel->setText(tr("🗂️ %1 个折叠").arg(count));
    } else {
        m_foldCountLabel->setText("");
    }
}

void MainWindow::foldAll()
{
    EditorWidget *editor = currentEditor();
    if (editor) editor->foldAll();
}

void MainWindow::unfoldAll()
{
    EditorWidget *editor = currentEditor();
    if (editor) editor->unfoldAll();
}