#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>

// 前置声明
class QLineEdit;
class QCheckBox;
class QPushButton;
class QPlainTextEdit;

class FindReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    // 构造函数需要传入要操作的编辑器
    explicit FindReplaceDialog(QPlainTextEdit *editor, QWidget *parent = nullptr);

    // 切换目标编辑器（因为多标签，当前编辑器可能会变）
    void setEditor(QPlainTextEdit *editor);

private slots:
    void onFindNext();       // 查找下一个
    void onFindPrevious();   // 查找上一个
    void onReplace();        // 替换当前
    void onReplaceAll();     // 全部替换

private:
    void setupUi();          // 构建界面

    // 内部辅助
    bool doFind(bool forward);

    QPlainTextEdit *m_editor;    // 目标编辑器

    // UI 控件
    QLineEdit *m_findLineEdit;
    QLineEdit *m_replaceLineEdit;
    QCheckBox *m_caseSensitiveCheckBox;
    QCheckBox *m_wholeWordCheckBox;
    QCheckBox *m_regexCheckBox;
    QPushButton *m_findNextButton;
    QPushButton *m_findPreviousButton;
    QPushButton *m_replaceButton;
    QPushButton *m_replaceAllButton;
};

#endif // FINDREPLACEDIALOG_H