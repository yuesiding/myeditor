#include "mindmapaidialog.h"
#include "thememanager.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>

MindMapAiDialog::MindMapAiDialog(const QString &parentTopic, QWidget *parent)
    : QDialog(parent)
    , m_parentTopic(parentTopic)
{
    setWindowTitle(tr("🤖 AI 建议"));
    setMinimumSize(500, 450);

    setupUi();
    applyTheme();

    // 初始为加载状态
    setLoading(true);
}

void MindMapAiDialog::setupUi()
{
    // 标题
    m_titleLabel = new QLabel(
        tr("为「%1」生成子话题建议").arg(m_parentTopic), this);
    m_titleLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; padding: 8px; }");

    // 状态
    m_statusLabel = new QLabel(tr("⏳ AI 正在思考..."), this);
    m_statusLabel->setStyleSheet("QLabel { padding: 4px; color: #666; }");

    // 进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);   // 不确定进度
    m_progressBar->setMaximumHeight(4);
    m_progressBar->setTextVisible(false);

    // 列表
    m_suggestionList = new QListWidget(this);
    m_suggestionList->setStyleSheet(
        "QListWidget::item { padding: 8px; }"
        "QListWidget::item:hover { background: #E3F2FD; }");

    // 选择按钮
    m_selectAllButton = new QPushButton(tr("全选"), this);
    m_selectNoneButton = new QPushButton(tr("全不选"), this);
    connect(m_selectAllButton, &QPushButton::clicked, this, &MindMapAiDialog::onSelectAll);
    connect(m_selectNoneButton, &QPushButton::clicked, this, &MindMapAiDialog::onSelectNone);

    // 确定/取消
    m_okButton = new QPushButton(tr("✅ 添加选中的节点"), this);
    m_okButton->setDefault(true);
    m_cancelButton = new QPushButton(tr("取消"), this);
    connect(m_okButton, &QPushButton::clicked, this, &MindMapAiDialog::onAcceptClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    // 布局
    QHBoxLayout *selectLayout = new QHBoxLayout;
    selectLayout->addWidget(m_selectAllButton);
    selectLayout->addWidget(m_selectNoneButton);
    selectLayout->addStretch();

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_okButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addWidget(m_statusLabel);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addWidget(m_suggestionList, 1);
    mainLayout->addLayout(selectLayout);
    mainLayout->addLayout(buttonLayout);
}

void MindMapAiDialog::applyTheme()
{
    // 简单适配，不做太多定制
}

void MindMapAiDialog::setSuggestions(const QStringList &topics)
{
    setLoading(false);
    m_suggestionList->clear();

    for (const QString &topic : topics) {
        QListWidgetItem *item = new QListWidgetItem(topic);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);   // 默认勾选
        m_suggestionList->addItem(item);
    }

    if (topics.isEmpty()) {
        m_statusLabel->setText(tr("😅 AI 没有返回建议"));
    } else {
        m_statusLabel->setText(tr("✨ AI 生成了 %1 个建议，勾选你想要的").arg(topics.size()));
    }
}

void MindMapAiDialog::setLoading(bool loading)
{
    m_progressBar->setVisible(loading);
    m_suggestionList->setVisible(!loading);
    m_selectAllButton->setEnabled(!loading);
    m_selectNoneButton->setEnabled(!loading);
    m_okButton->setEnabled(!loading);

    if (loading) {
        m_statusLabel->setText(tr("⏳ AI 正在思考..."));
    }
}

void MindMapAiDialog::showError(const QString &error)
{
    setLoading(false);
    m_statusLabel->setText(tr("❌ %1").arg(error));
    m_okButton->setEnabled(false);
}

QStringList MindMapAiDialog::selectedTopics() const
{
    QStringList result;
    for (int i = 0; i < m_suggestionList->count(); ++i) {
        QListWidgetItem *item = m_suggestionList->item(i);
        if (item->checkState() == Qt::Checked) {
            result.append(item->text());
        }
    }
    return result;
}

void MindMapAiDialog::onAcceptClicked()
{
    accept();
}

void MindMapAiDialog::onSelectAll()
{
    for (int i = 0; i < m_suggestionList->count(); ++i) {
        m_suggestionList->item(i)->setCheckState(Qt::Checked);
    }
}

void MindMapAiDialog::onSelectNone()
{
    for (int i = 0; i < m_suggestionList->count(); ++i) {
        m_suggestionList->item(i)->setCheckState(Qt::Unchecked);
    }
}