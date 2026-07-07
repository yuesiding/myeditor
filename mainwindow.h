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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void openFileByPath(const QString &filePath);

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
        // 🆕 折叠数量变化
    void updateFoldCount(int count);

    // 🆕 折叠菜单
    void foldAll();
    void unfoldAll();
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
};

#endif // MAINWINDOW_H