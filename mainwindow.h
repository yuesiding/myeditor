#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>

class QLabel;
class QTabWidget;
class EditorWidget;
class FindReplaceDialog;  
class QMenu;   
class QDragEnterEvent;
class QDropEvent;
class QToolBar;
class QDockWidget;
class FileTreeWidget;
class TaskListWidget;
class CommandPalette;
class AiAssistantWidget;
class TerminalWidget;
class QStackedWidget;
class QToolBar;
class MindMapView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void openFileByPath(const QString &filePath);
        //  供 EditorWidget 调用：让 AI 处理选中的代码
    void askAiAboutCode(const QString &prompt, const QString &code);
protected:
    void closeEvent(QCloseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveAsFile();
    void closeTab(int index);
    void updateTabTitle();
    void onCurrentTabChanged(int index);
        
    void updateStatusBarInfo();// 状态栏更新

    // 查找替换
    void showFindReplaceDialog();
    void switchToLightTheme();
    void switchToDarkTheme();   
    void openRecentFile();
    void clearRecentFiles();// 最近文件
        
    void zoomIn();//  字体缩放
    void zoomOut();
    void resetZoom();
        
    void undo();//  撤销重做
    void redo();

    void showPreferences();
    void applySettingsToAllEditors();

    void toggleFileTree();
    void openFolderInTree();
    void onFileTreeDoubleClicked(const QString &filePath);
       
    void updateFoldCount(int count); // 🆕 折叠数量变化
    
    void foldAll();// 🆕 折叠菜单
    void unfoldAll();
        
    void toggleTaskList();// 🆕 任务清单
        
    void updateTaskCount(int total, int completed);// 🆕 更新任务数量显示
        
    void showCommandPalette();// 命令面板
    void toggleAiAssistant();
    void runCurrentFile();
    void toggleTerminal();
       
    void switchToEditorMode(); // 🆕 模式切换
    void switchToMindMapMode();
        // 🆕 思维导图文件操作
    void newMindMap();
    void openMindMap();
    void saveMindMap();
    void saveAsMindMap();

    // 🆕 更新窗口标题（根据当前模式和文件）
    void updateWindowTitleForMindMap();
        // 🆕 思维导图的撤销/重做
    void mindMapUndo();
    void mindMapRedo();
    void exportMindMapImage();
private:
    void createMenus();
    EditorWidget *createEditor();
    EditorWidget *currentEditor() const;

    QTabWidget *m_tabWidget;
    FindReplaceDialog *m_findReplaceDialog;   
    QLabel *m_cursorPositionLabel;   // 行:X 列:Y
    QLabel *m_encodingLabel;         // 编码
    QLabel *m_charCountLabel;        // 字符数

    void loadRecentFiles();          // 从 QSettings 加载
    void saveRecentFiles();          // 保存到 QSettings
    void addRecentFile(const QString &filePath);  // 添加一个
    void updateRecentFilesMenu();    // 刷新菜单显示
    void createToolBars();// 创建工具栏
    void saveSession(); //  会话保存/恢复
    void restoreSession();
    QStringList m_recentFiles;       // 最近文件列表（最多 10 个）
    QMenu *m_recentFilesMenu;        // 最近文件子菜单
    static const int MaxRecentFiles = 10;
    QToolBar *m_fileToolBar;
    QToolBar *m_editToolBar;
        // 🆕 文件树
    QDockWidget *m_fileTreeDock;
    FileTreeWidget *m_fileTreeWidget;
    QAction *m_toggleFileTreeAction;   // 显示/隐藏菜单项
        
    QLabel *m_foldCountLabel;// 🆕 折叠数量标签

    QDockWidget *m_taskListDock;// 🆕 任务清单
    TaskListWidget *m_taskListWidget;
    QAction *m_toggleTaskListAction;

    QLabel *m_taskCountLabel; // 🆕 任务清单标签
        // 🆕 命令面板
    void registerCommands();
    CommandPalette *m_commandPalette;
    QDockWidget *m_aiDock;
    AiAssistantWidget *m_aiAssistantWidget;
    QAction *m_toggleAiAction;
    QDockWidget *m_terminalDock;
    TerminalWidget *m_terminalWidget;
    QAction *m_toggleTerminalAction;
    QAction *m_runAction;
        // 🆕 应用模式栈
    void createModeSwitcher();
    
    QWidget *m_editorModeWidget;// 🆕 编辑器模式的中心 widget

    MindMapView *m_mindMapView;// 🆕 思维导图模式

    QStackedWidget *m_modeStack;// 🆕 模式栈（切换编辑器/思维导图）

    QToolBar *m_modeToolBar; // 🆕 模式切换工具栏
    QAction *m_editorModeAction;
    QAction *m_mindMapModeAction;
public slots:
    // 🆕 供思维导图节点调用（用 QMetaObject::invokeMethod）
    public slots:
    void requestAiExpandFromNode(QVariant nodePtr);
};

#endif // MAINWINDOW_H