#include "mainwindow.h"
#include "editorwidget.h"
#include "findreplacedialog.h"
#include "thememanager.h"
#include "preferencesdialog.h"
#include "tasklistwidget.h"
#include "aiassistantwidget.h"
#include "thememanager.h"
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
#include <QToolBar>
#include <QStyle>
#include <QIcon>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent), m_findReplaceDialog(nullptr)
{
    //初始化状态栏label
    m_cursorPositionLabel = new QLabel(tr("行:1 列:1"), this);
    m_encodingLabel = new QLabel(tr("UTF-8"), this);
    m_charCountLabel = new QLabel(tr("0 字符"), this);
    m_foldCountLabel = new QLabel(tr(""), this);
    m_taskCountLabel = new QLabel(tr(""), this);

    m_cursorPositionLabel->setMinimumWidth(120);
    m_encodingLabel->setMinimumWidth(60);
    m_charCountLabel->setMinimumWidth(100);
    m_foldCountLabel->setMinimumWidth(100);
    m_taskCountLabel->setMinimumWidth(120);

    statusBar()->addPermanentWidget(m_cursorPositionLabel);
    statusBar()->addPermanentWidget(m_encodingLabel);
    statusBar()->addPermanentWidget(m_charCountLabel);
    statusBar()->addPermanentWidget(m_foldCountLabel);
    statusBar()->addPermanentWidget(m_taskCountLabel);
    //创建标签页
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);

    connect(m_tabWidget, &QTabWidget::tabCloseRequested,
            this, &MainWindow::closeTab);
    connect(m_tabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::onCurrentTabChanged);
        setCentralWidget(m_tabWidget);

    //创建菜单和工具栏
    createMenus();
    createToolBars();

    //创建任务清单dock
    m_taskListWidget = new TaskListWidget(this);
    m_taskListDock = new QDockWidget(tr("任务清单"), this);
    m_taskListDock->setObjectName("taskListDock");
    m_taskListDock->setWidget(m_taskListWidget);
    m_taskListDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_taskListDock);

    connect(m_taskListWidget, &TaskListWidget::taskCountChanged,
            this, &MainWindow::updateTaskCount);

    //创建AI助手dock
    m_aiAssistantWidget = new AiAssistantWidget(this);
    m_aiDock = new QDockWidget(tr("AI 助手"), this);
    m_aiDock->setObjectName("aiDock");
    m_aiDock->setWidget(m_aiAssistantWidget);
    m_aiDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_aiDock);
    tabifyDockWidget(m_taskListDock, m_aiDock);
    m_taskListDock->raise();
    updateTaskCount(m_taskListWidget->totalCount(),
                    m_taskListWidget->completedCount()); //更新任务数量
    createEditor(); //创建初始编辑器
    
    resize(1200, 800);  // 窗口设置
    setWindowTitle(tr("CodeEditor"));

    restoreSession();//恢复会话
}

MainWindow::~MainWindow(){}

void MainWindow::createMenus(){
    //文件菜单
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
    fileMenu->addSeparator();

    QAction *exitAction = new QAction(tr("退出(&X)"), this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(exitAction);
    //视图菜单
    QMenu *viewMenu = menuBar()->addMenu(tr("视图(&V)"));
    QMenu *themeMenu = viewMenu->addMenu(tr("主题(&T)"));
    //字体缩放
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
    //编辑菜单
    QMenu *editMenu = menuBar()->addMenu(tr("编辑(&E)"));
    QAction *findAction = new QAction(tr("查找和替换(&F)..."), this);
    findAction->setShortcut(QKeySequence::Find);   // Ctrl+F
    connect(findAction, &QAction::triggered, this, &MainWindow::showFindReplaceDialog);
    editMenu->addAction(findAction);

    //显示/隐藏任务清单或 AI 助手
    m_toggleTaskListAction = new QAction(tr("显示任务清单(&T)"), this);
    m_toggleTaskListAction->setCheckable(true);
    m_toggleTaskListAction->setChecked(true);
    m_toggleTaskListAction->setShortcut(tr("Ctrl+T"));
    connect(m_toggleTaskListAction, &QAction::triggered,
            this, &MainWindow::toggleTaskList);
    viewMenu->addAction(m_toggleTaskListAction);
    m_toggleAiAction = new QAction(tr("显示 AI 助手(&I)"), this);
    m_toggleAiAction->setCheckable(true);
    m_toggleAiAction->setChecked(true);
    m_toggleAiAction->setShortcut(tr("Ctrl+I"));
    connect(m_toggleAiAction, &QAction::triggered,
            this, &MainWindow::toggleAiAssistant);
    viewMenu->addAction(m_toggleAiAction);
    //折叠菜单
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

EditorWidget *MainWindow::createEditor(){
    EditorWidget *editor = new EditorWidget(this);
    int index = m_tabWidget->addTab(editor, editor->userFriendlyName());
    m_tabWidget->setCurrentIndex(index);

    connect(editor, &EditorWidget::fileInfoChanged,
            this, &MainWindow::updateTabTitle);

    return editor;
}

EditorWidget *MainWindow::currentEditor() const{
    return qobject_cast<EditorWidget *>(m_tabWidget->currentWidget());
}

void MainWindow::newFile(){
    createEditor();
}

void MainWindow::openFile(){
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"));
    openFileByPath(fileName);
}
void MainWindow::saveFile(){
    EditorWidget *editor = currentEditor();
    if (editor) {
        editor->save();
    }
}

void MainWindow::saveAsFile(){
    EditorWidget *editor = currentEditor();
    if (!editor) return;

    QString fileName = QFileDialog::getSaveFileName(this, tr("另存为"));
    if (fileName.isEmpty()) return;

    editor->saveAs(fileName);
}

void MainWindow::closeTab(int index){
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

void MainWindow::updateTabTitle(){
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

void MainWindow::onCurrentTabChanged(int index){
    Q_UNUSED(index);
    EditorWidget *editor = currentEditor();
    if (editor) {
        QString name = editor->currentFile().isEmpty() ?
                       tr("未命名") : editor->currentFile();
        statusBar()->showMessage(name);   
        //连接光标和内容变化信号
        disconnect(editor, &QPlainTextEdit::cursorPositionChanged,
                   this, &MainWindow::updateStatusBarInfo);
        disconnect(editor, &QPlainTextEdit::textChanged,
                   this, &MainWindow::updateStatusBarInfo);

        connect(editor, &QPlainTextEdit::cursorPositionChanged,
                this, &MainWindow::updateStatusBarInfo);
        connect(editor, &QPlainTextEdit::textChanged,
                this, &MainWindow::updateStatusBarInfo);
            //连接折叠数量变化
    disconnect(editor, &EditorWidget::foldCountChanged,
               this, &MainWindow::updateFoldCount);
    connect(editor, &EditorWidget::foldCountChanged,
            this, &MainWindow::updateFoldCount);
    updateFoldCount(editor->foldedCount());// 立即更新一次
    updateStatusBarInfo();// 立即更新一次
    if (m_findReplaceDialog) {
            m_findReplaceDialog->setEditor(editor);
        }
    }else{
        statusBar()->clearMessage();
    }
}

//显示查找替换对话框
void MainWindow::showFindReplaceDialog(){
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

void MainWindow::closeEvent(QCloseEvent *event){

    //询问未保存的文件
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

//主题切换
void MainWindow::switchToLightTheme(){
    ThemeManager::instance().setLightTheme();
}

void MainWindow::switchToDarkTheme(){
    ThemeManager::instance().setDarkTheme();
}

//更新状态栏信息
void MainWindow::updateStatusBarInfo(){
    if (!m_cursorPositionLabel||!m_encodingLabel||!m_charCountLabel){
        return;
    }
    EditorWidget *editor = currentEditor();
    if (!editor) return;
    // 获取光标位置
    QTextCursor cursor = editor->textCursor();
    int line = cursor.blockNumber() + 1; 
    int column = cursor.columnNumber() + 1;
    // 更新行列号
    m_cursorPositionLabel->setText(tr("行:%1 列:%2").arg(line).arg(column));
    // 更新字符数
    int charCount = editor->document()->characterCount()-1;
    m_charCountLabel->setText(tr("%1 字符").arg(charCount));
}

//打开文件
void MainWindow::openFileByPath(const QString &filePath){
    if (filePath.isEmpty()) return;
    if (!QFileInfo::exists(filePath)) {
        QMessageBox::warning(this, tr("警告"),tr("文件不存在：%1").arg(filePath));
        return;
    }
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        EditorWidget *editor = qobject_cast<EditorWidget *>(m_tabWidget->widget(i));
        if (editor && editor->currentFile() == filePath) {
            m_tabWidget->setCurrentIndex(i);
            return;
        }
    }
    EditorWidget *editor = currentEditor();
    if (!editor || !editor->currentFile().isEmpty() || editor->isModified()){
        editor = createEditor();
    }
    if(editor->loadFile(filePath)){
    }else{
        int idx=m_tabWidget->indexOf(editor);
        if(idx>=0) {
            m_tabWidget->removeTab(idx);
            editor->deleteLater();
        }
    }
}
//字体缩放
void MainWindow::zoomIn(){
    EditorWidget *editor = currentEditor();
    if (editor) editor->zoomIn();
}

void MainWindow::zoomOut(){
    EditorWidget *editor = currentEditor();
    if (editor) editor->zoomOut();
}

void MainWindow::resetZoom(){
    EditorWidget *editor = currentEditor();
    if (editor) editor->resetZoom();
}
//创建工具栏
void MainWindow::createToolBars(){
    QStyle *style = this->style();
    //文件工具栏
    m_fileToolBar = addToolBar(tr("文件"));
    m_fileToolBar->setObjectName("fileToolBar");   // 用于保存状态

    QAction *newAct = m_fileToolBar->addAction(style->standardIcon(QStyle::SP_FileIcon),tr("新建"));
    newAct->setShortcut(QKeySequence::New);
    newAct->setToolTip(tr("新建文件 (Ctrl+N)"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    QAction *openAct = m_fileToolBar->addAction(style->standardIcon(QStyle::SP_DialogOpenButton),tr("打开"));
    openAct->setShortcut(QKeySequence::Open);
    openAct->setToolTip(tr("打开文件 (Ctrl+O)"));
    connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

    QAction *saveAct = m_fileToolBar->addAction(style->standardIcon(QStyle::SP_DialogSaveButton),tr("保存"));
    saveAct->setShortcut(QKeySequence::Save);
    saveAct->setToolTip(tr("保存文件 (Ctrl+S)"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);
    //编辑工具栏
    m_editToolBar = addToolBar(tr("编辑"));
    m_editToolBar->setObjectName("editToolBar");

    QAction *undoAct = m_editToolBar->addAction(style->standardIcon(QStyle::SP_ArrowBack),tr("撤销"));
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setToolTip(tr("撤销 (Ctrl+Z)"));
    connect(undoAct, &QAction::triggered, this, &MainWindow::undo);

    QAction *redoAct = m_editToolBar->addAction(style->standardIcon(QStyle::SP_ArrowForward),tr("重做"));
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setToolTip(tr("重做 (Ctrl+Y)"));
    connect(redoAct, &QAction::triggered, this, &MainWindow::redo);

    m_editToolBar->addSeparator();

    QAction *findAct = m_editToolBar->addAction(style->standardIcon(QStyle::SP_FileDialogContentsView),tr("查找"));
    findAct->setShortcut(QKeySequence::Find);
    findAct->setToolTip(tr("查找和替换 (Ctrl+F)"));
    connect(findAct, &QAction::triggered, this, &MainWindow::showFindReplaceDialog);

}

void MainWindow::undo(){
    EditorWidget *editor = currentEditor();
    if (editor) editor->undo();
}

void MainWindow::redo(){
    EditorWidget *editor = currentEditor();
    if (editor) editor->redo();
}
void MainWindow::saveSession()
{
    QSettings settings("MyCompany", "CodeEditor");
    settings.setValue("geometry", saveGeometry());      
    settings.setValue("windowState", saveState());      

    QStringList openFiles; //保存打开的文件
    QList<int> cursorPositions;

    for (int i=0; i<m_tabWidget->count(); ++i) {
        EditorWidget *editor=qobject_cast<EditorWidget *>(m_tabWidget->widget(i));
        if (editor&&!editor->currentFile().isEmpty()){
            openFiles.append(editor->currentFile());
            cursorPositions.append(editor->cursorPosition());
        }
    }
    settings.setValue("session/openFiles", openFiles);
    QVariantList cursorList; // QSettings不直接支持QList<int>，只得转成QVariantList
    for (int pos : cursorPositions) {
        cursorList.append(pos);
    }
    settings.setValue("session/cursorPositions", cursorList);
    settings.setValue("session/currentIndex", m_tabWidget->currentIndex());
        
}

void MainWindow::restoreSession(){
    QSettings settings("MyCompany", "CodeEditor");
    QByteArray geometry = settings.value("geometry").toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    }
    QByteArray windowState = settings.value("windowState").toByteArray();
    if (!windowState.isEmpty()) {
        restoreState(windowState);
    }
    QStringList openFiles = settings.value("session/openFiles").toStringList();
    QVariantList cursorList = settings.value("session/cursorPositions").toList();
    if (openFiles.isEmpty()) return;  
    while (m_tabWidget->count()>0) {
        QWidget *w = m_tabWidget->widget(0);
        m_tabWidget->removeTab(0);
        w->deleteLater();
    }
    for (int i=0; i<openFiles.size();++i) {
        QString filePath=openFiles.at(i);
        if (!QFileInfo::exists(filePath)) {
            continue;  
        }
        EditorWidget *editor=createEditor();
        if (editor->loadFile(filePath)) {
            if (i<cursorList.size()) {
                int pos=cursorList.at(i).toInt();
                editor->setCursorPosition(pos);
            }
        }
    }
    if (m_tabWidget->count()==0) {
        createEditor();
    }
    int currentIndex=settings.value("session/currentIndex", 0).toInt();
    if (currentIndex>=0&&currentIndex<m_tabWidget->count()) {
        m_tabWidget->setCurrentIndex(currentIndex);
    }
}
//首选项
void MainWindow::showPreferences()
{
    PreferencesDialog dialog(this);
    connect(&dialog, &PreferencesDialog::settingsApplied,this, &MainWindow::applySettingsToAllEditors);
    dialog.exec();
}
void MainWindow::applySettingsToAllEditors()
{
    for (int i=0;i<m_tabWidget->count();++i) {
        EditorWidget *editor = qobject_cast<EditorWidget *>(m_tabWidget->widget(i));
        if (editor) {
            editor->applySettings();
        }
    }
}
//折叠相关
void MainWindow::updateFoldCount(int count)
{
    if (count>0) {
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

void MainWindow::toggleTaskList()
{
    if(m_taskListDock->isVisible()) {
        m_taskListDock->hide();
        m_toggleTaskListAction->setChecked(false);
    }else{
        m_taskListDock->show();
        m_toggleTaskListAction->setChecked(true);
    }
}

void MainWindow::updateTaskCount(int total, int completed)
{
    int pending=total-completed;
    if(total==0) {
        m_taskCountLabel->setText(tr(""));
    }else{
        m_taskCountLabel->setText(
            tr("📋 %1 待办 / %2 完成").arg(pending).arg(completed)
        );
    }
}
//AI
void MainWindow::toggleAiAssistant()
{
    if (m_aiDock->isVisible()) {
        m_aiDock->hide();
        m_toggleAiAction->setChecked(false);
    } else {
        m_aiDock->show();
        m_aiDock->raise();
        m_toggleAiAction->setChecked(true);
    }
}

//AI代码处理接口，，参考https://github.com/ChengYull/Qt-ChatTool
void MainWindow::askAiAboutCode(const QString &prompt, const QString &code)
{
    if (!m_aiAssistantWidget) return;
    m_aiDock->show();
    m_aiDock->raise();
    m_toggleAiAction->setChecked(true);
    if (prompt.isEmpty() && code.isEmpty()) {
        return;
    }
    QString fullQuestion;
    if(code.isEmpty()) {
        fullQuestion = prompt;
    }else{
        fullQuestion = QString("%1\n\n```\n%2\n```").arg(prompt, code);
    }
    m_aiAssistantWidget->askQuestion(fullQuestion);
}
