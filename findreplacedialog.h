#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QPushButton;
class QPlainTextEdit;

class FindReplaceDialog:public QDialog
{
    Q_OBJECT

public:
    explicit FindReplaceDialog(QPlainTextEdit *editor, QWidget *parent=nullptr);
    void setEditor(QPlainTextEdit *editor);
private slots:
    void onFindNext();       
    void onFindPrevious();   
    void onReplace();       
    void onReplaceAll();     
private:
    void setupUi();      
    bool doFind(bool forward);
    QPlainTextEdit *m_editor;  
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