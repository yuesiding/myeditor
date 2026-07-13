#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

class QListWidget;
class QStackedWidget;
class QFontComboBox;
class QSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;

class PreferencesDialog:public QDialog
{
    Q_OBJECT
public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
signals:
    void settingsApplied();
private slots:
    void onOkClicked();
    void onApplyClicked();
private:
    void setupUi();
    void createEditorPage();
    void createThemePage();
    void loadSettings();   
    void saveSettings();   
    //左侧分类
    QListWidget *m_categoryList;
    //右侧内容
    QStackedWidget *m_pagesStack;
    //编辑器
    QWidget *m_editorPage;
    QFontComboBox *m_fontComboBox;
    QSpinBox *m_fontSizeSpin;
    QCheckBox *m_showLineNumbersCheck;
    QCheckBox *m_highlightCurrentLineCheck;
    QCheckBox *m_matchBracketsCheck;
    QSpinBox *m_tabSizeSpin;
    //主题
    QWidget *m_themePage;
    QComboBox *m_themeCombo;
    //按钮
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QPushButton *m_applyButton;
};

#endif // PREFERENCESDIALOG_H