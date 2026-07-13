#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringList>

class QLabel;
class QTabWidget;
class EditorWidget;
class FindReplaceDialog;
class QMenu;
class QToolBar;
class QDockWidget;
class TaskListWidget;
class AiAssistantWidget;


class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void openFileByPath(const QString &filePath);
    void askAiAboutCode(const QString &prompt, const QString &code);
protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    //文件操作
    void newFile();
    void openFile();
    void saveFile();
    void saveAsFile();
    void closeTab(int index);
    void updateTabTitle();
    void onCurrentTabChanged(int index);
    //状态栏更新
    void updateStatusBarInfo();
    //查找替换
    void showFindReplaceDialog();
    //主题
    void switchToLightTheme();
    void switchToDarkTheme();
    //字体缩放
    void zoomIn();
    void zoomOut();
    void resetZoom();
    //撤销重做
    void undo();
    void redo();
    //首选项
    void showPreferences();
    void applySettingsToAllEditors();
    //折叠
    void updateFoldCount(int count);
    void foldAll();
    void unfoldAll();
    //任务清单
    void toggleTaskList();
    void updateTaskCount(int total, int completed);
    //AI
    void toggleAiAssistant();
private:
    void createMenus();
    void createToolBars();
    EditorWidget *createEditor();
    EditorWidget *currentEditor() const;
    // 会话保存/恢复
    void saveSession();
    void restoreSession();
    QTabWidget *m_tabWidget;
    FindReplaceDialog *m_findReplaceDialog;
    //状态栏
    QLabel *m_cursorPositionLabel;
    QLabel *m_encodingLabel;
    QLabel *m_charCountLabel;
    QLabel *m_foldCountLabel;
    QLabel *m_taskCountLabel;
    //工具栏
    QToolBar *m_fileToolBar;
    QToolBar *m_editToolBar;
    //任务清单
    QDockWidget *m_taskListDock;
    TaskListWidget *m_taskListWidget;
    QAction *m_toggleTaskListAction;
    //AI
    QDockWidget *m_aiDock;
    AiAssistantWidget *m_aiAssistantWidget;
    QAction *m_toggleAiAction;
};

#endif // MAINWINDOW_H