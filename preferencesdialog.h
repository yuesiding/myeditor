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

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);

signals:
    // 设置被应用时发出（让编辑器重新读取）
    void settingsApplied();

private slots:
    void onOkClicked();
    void onApplyClicked();

private:
    void setupUi();
    void createEditorPage();
    void createThemePage();

    void loadSettings();   // 从 QSettings 加载到控件
    void saveSettings();   // 从控件保存到 QSettings

    // 左侧分类
    QListWidget *m_categoryList;
    // 右侧内容
    QStackedWidget *m_pagesStack;

    // 编辑器页控件
    QWidget *m_editorPage;
    QFontComboBox *m_fontComboBox;
    QSpinBox *m_fontSizeSpin;
    QCheckBox *m_showLineNumbersCheck;
    QCheckBox *m_highlightCurrentLineCheck;
    QCheckBox *m_matchBracketsCheck;
    QSpinBox *m_tabSizeSpin;

    // 主题页控件
    QWidget *m_themePage;
    QComboBox *m_themeCombo;

    // 底部按钮
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
    QPushButton *m_applyButton;
};

#endif // PREFERENCESDIALOG_H