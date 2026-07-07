#include "preferencesdialog.h"
#include "thememanager.h"

#include <QListWidget>
#include <QStackedWidget>
#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSettings>

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("首选项"));
    setMinimumSize(600, 400);

    setupUi();
    loadSettings();
}

void PreferencesDialog::setupUi()
{
    // ===== 左侧分类列表 =====
    m_categoryList = new QListWidget(this);
    m_categoryList->addItem(tr("编辑器"));
    m_categoryList->addItem(tr("主题"));
    m_categoryList->setCurrentRow(0);
    m_categoryList->setMaximumWidth(150);

    // ===== 右侧内容堆叠 =====
    m_pagesStack = new QStackedWidget(this);
    createEditorPage();
    createThemePage();
    m_pagesStack->addWidget(m_editorPage);
    m_pagesStack->addWidget(m_themePage);

    // 切换分类时切换页面
    connect(m_categoryList, &QListWidget::currentRowChanged,
            m_pagesStack, &QStackedWidget::setCurrentIndex);

    // ===== 底部按钮 =====
    m_okButton = new QPushButton(tr("确定"), this);
    m_cancelButton = new QPushButton(tr("取消"), this);
    m_applyButton = new QPushButton(tr("应用"), this);

    m_okButton->setDefault(true);

    connect(m_okButton, &QPushButton::clicked, this, &PreferencesDialog::onOkClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_applyButton, &QPushButton::clicked, this, &PreferencesDialog::onApplyClicked);

    // ===== 布局 =====
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(m_categoryList);
    topLayout->addWidget(m_pagesStack, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_applyButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout, 1);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);
}

void PreferencesDialog::createEditorPage()
{
    m_editorPage = new QWidget(this);

    // ===== 字体组 =====
    QGroupBox *fontGroup = new QGroupBox(tr("字体设置"), m_editorPage);
    QFormLayout *fontLayout = new QFormLayout(fontGroup);

    m_fontComboBox = new QFontComboBox(fontGroup);
    // 只显示等宽字体（更适合代码编辑）
    m_fontComboBox->setFontFilters(QFontComboBox::MonospacedFonts);
    fontLayout->addRow(tr("字体名称:"), m_fontComboBox);

    m_fontSizeSpin = new QSpinBox(fontGroup);
    m_fontSizeSpin->setRange(6, 40);
    m_fontSizeSpin->setSuffix(tr(" pt"));
    fontLayout->addRow(tr("字体大小:"), m_fontSizeSpin);

    // ===== 编辑选项组 =====
    QGroupBox *editGroup = new QGroupBox(tr("编辑选项"), m_editorPage);
    QVBoxLayout *editLayout = new QVBoxLayout(editGroup);

    m_showLineNumbersCheck = new QCheckBox(tr("显示行号"), editGroup);
    m_highlightCurrentLineCheck = new QCheckBox(tr("高亮当前行"), editGroup);
    m_matchBracketsCheck = new QCheckBox(tr("括号匹配"), editGroup);

    editLayout->addWidget(m_showLineNumbersCheck);
    editLayout->addWidget(m_highlightCurrentLineCheck);
    editLayout->addWidget(m_matchBracketsCheck);

    // Tab 大小
    QHBoxLayout *tabLayout = new QHBoxLayout;
    tabLayout->addWidget(new QLabel(tr("Tab 大小:"), editGroup));
    m_tabSizeSpin = new QSpinBox(editGroup);
    m_tabSizeSpin->setRange(1, 16);
    m_tabSizeSpin->setSuffix(tr(" 空格"));
    tabLayout->addWidget(m_tabSizeSpin);
    tabLayout->addStretch();
    editLayout->addLayout(tabLayout);

    // 页面布局
    QVBoxLayout *layout = new QVBoxLayout(m_editorPage);
    layout->addWidget(fontGroup);
    layout->addWidget(editGroup);
    layout->addStretch();
}

void PreferencesDialog::createThemePage()
{
    m_themePage = new QWidget(this);

    QGroupBox *themeGroup = new QGroupBox(tr("主题选择"), m_themePage);
    QFormLayout *themeLayout = new QFormLayout(themeGroup);

    m_themeCombo = new QComboBox(themeGroup);
    m_themeCombo->addItem(tr("亮色主题"), "light");
    m_themeCombo->addItem(tr("暗色主题"), "dark");
    themeLayout->addRow(tr("当前主题:"), m_themeCombo);

    QVBoxLayout *layout = new QVBoxLayout(m_themePage);
    layout->addWidget(themeGroup);
    layout->addStretch();
}

void PreferencesDialog::loadSettings()
{
    QSettings settings("MyCompany", "CodeEditor");

    // 编辑器设置
    QString fontName = settings.value("editor/fontName", "Consolas").toString();
    m_fontComboBox->setCurrentFont(QFont(fontName));

    int fontSize = settings.value("editor/fontSize", 11).toInt();
    m_fontSizeSpin->setValue(fontSize);

    m_showLineNumbersCheck->setChecked(
        settings.value("editor/showLineNumbers", true).toBool());
    m_highlightCurrentLineCheck->setChecked(
        settings.value("editor/highlightCurrentLine", true).toBool());
    m_matchBracketsCheck->setChecked(
        settings.value("editor/matchBrackets", true).toBool());

    m_tabSizeSpin->setValue(settings.value("editor/tabSize", 4).toInt());

    // 主题设置
    QString theme = settings.value("theme/current", "light").toString();
    int idx = m_themeCombo->findData(theme);
    if (idx >= 0) {
        m_themeCombo->setCurrentIndex(idx);
    }
}

void PreferencesDialog::saveSettings()
{
    QSettings settings("MyCompany", "CodeEditor");

    // 编辑器设置
    settings.setValue("editor/fontName", m_fontComboBox->currentFont().family());
    settings.setValue("editor/fontSize", m_fontSizeSpin->value());
    settings.setValue("editor/showLineNumbers", m_showLineNumbersCheck->isChecked());
    settings.setValue("editor/highlightCurrentLine", m_highlightCurrentLineCheck->isChecked());
    settings.setValue("editor/matchBrackets", m_matchBracketsCheck->isChecked());
    settings.setValue("editor/tabSize", m_tabSizeSpin->value());

    // 主题设置
    QString theme = m_themeCombo->currentData().toString();
    settings.setValue("theme/current", theme);
    if (theme == "dark") {
        ThemeManager::instance().setDarkTheme();
    } else {
        ThemeManager::instance().setLightTheme();
    }
}

void PreferencesDialog::onOkClicked()
{
    saveSettings();
    emit settingsApplied();
    accept();
}

void PreferencesDialog::onApplyClicked()
{
    saveSettings();
    emit settingsApplied();
}