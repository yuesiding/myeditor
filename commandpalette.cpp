#include "commandpalette.h"
#include "thememanager.h"

#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QScreen>
#include <QApplication>
#include <QGraphicsDropShadowEffect>

CommandPalette::CommandPalette(QWidget *parent)
    : QDialog(parent)
{
    // 无边框 + 半透明背景
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setupUi();

    // 🆕 监听主题变化
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this]() { applyTheme(); });

    // 大小
    resize(720, 500);

    // 应用初始主题
    applyTheme();
}

void CommandPalette::setupUi()
{
    // ===== 主容器（用于圆角和阴影）=====
    // 直接给 dialog 加样式即可

    // ===== 搜索框 =====
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("🔍  输入命令名称，回车执行..."));
    m_searchEdit->setObjectName("searchEdit");

    // ===== 命令列表 =====
    m_listWidget = new QListWidget(this);
    m_listWidget->setObjectName("commandList");
    m_listWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_listWidget->setUniformItemSizes(true);   // 提高性能
    m_listWidget->setSpacing(0);

    // ===== 布局 =====
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(1, 1, 1, 1);   // 留一点边距给圆角边框
    layout->setSpacing(0);
    layout->addWidget(m_searchEdit);
    layout->addWidget(m_listWidget);

    // 信号连接
    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &CommandPalette::onSearchTextChanged);
    connect(m_listWidget, &QListWidget::itemDoubleClicked,
            this, &CommandPalette::executeSelected);
    // 🆕 单击也能执行（更符合直觉）
    connect(m_listWidget, &QListWidget::itemClicked,
            this, &CommandPalette::executeSelected);
}

// 🆕 应用主题
void CommandPalette::applyTheme()
{
    const Theme &theme = ThemeManager::instance().currentTheme();
    bool isDark = theme.editorBackground.lightness() < 128;

    // 颜色变量
    QString bgColor, fgColor, borderColor, itemHoverBg, selectionBg, selectionFg, placeholderColor, shortcutColor;

    if (isDark) {
        // 暗主题（仿 VSCode Dark+）
        bgColor = "#252526";
        fgColor = "#CCCCCC";
        borderColor = "#454545";
        itemHoverBg = "#2A2D2E";
        selectionBg = "#094771";
        selectionFg = "#FFFFFF";
        placeholderColor = "#6A6A6A";
        shortcutColor = "#858585";
    } else {
        // 亮主题（仿 VSCode Light+）
        bgColor = "#F3F3F3";
        fgColor = "#333333";
        borderColor = "#C8C8C8";
        itemHoverBg = "#E8E8E8";
        selectionBg = "#0078D4";
        selectionFg = "#FFFFFF";
        placeholderColor = "#A0A0A0";
        shortcutColor = "#767676";
    }

    // 整体对话框样式
    setStyleSheet(QString(
        "CommandPalette { "
        "  background: %1; "
        "  border: 1px solid %2; "
        "  border-radius: 6px; "
        "} "

        "QLineEdit#searchEdit { "
        "  padding: 14px 18px; "
        "  font-size: 16px; "
        "  border: none; "
        "  border-bottom: 1px solid %2; "
        "  background: %1; "
        "  color: %3; "
        "} "

        "QLineEdit#searchEdit:focus { "
        "  outline: none; "
        "  border: none; "
        "  border-bottom: 1px solid %2; "
        "} "

        "QListWidget#commandList { "
        "  border: none; "
        "  background: %1; "
        "  color: %3; "
        "  outline: none; "
        "  padding: 4px 0px; "
        "} "

        "QListWidget#commandList::item { "
        "  padding: 10px 18px; "
        "  color: %3; "
        "  border: none; "
        "  min-height: 28px; "
        "} "

        "QListWidget#commandList::item:hover { "
        "  background: %4; "
        "} "

        "QListWidget#commandList::item:selected { "
        "  background: %5; "
        "  color: %6; "
        "} "

        "QScrollBar:vertical { "
        "  background: %1; "
        "  width: 10px; "
        "  border: none; "
        "} "
        "QScrollBar::handle:vertical { "
        "  background: %2; "
        "  border-radius: 3px; "
        "  min-height: 30px; "
        "} "
        "QScrollBar::handle:vertical:hover { "
        "  background: %4; "
        "} "
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { "
        "  height: 0px; "
        "}"
    ).arg(bgColor, borderColor, fgColor, itemHoverBg, selectionBg, selectionFg));

    // 保存快捷键颜色用于列表项渲染
    m_shortcutColor = shortcutColor;
    m_bgColor = bgColor;
    m_fgColor = fgColor;

    // 刷新列表以应用新颜色
    onSearchTextChanged(m_searchEdit ? m_searchEdit->text() : "");
}

void CommandPalette::addCommand(const Command &cmd)
{
    m_commands.append(cmd);
}

void CommandPalette::clearCommands()
{
    m_commands.clear();
    m_listWidget->clear();
}

void CommandPalette::showPalette()
{
    m_searchEdit->clear();
    onSearchTextChanged("");

    // 居中到父窗口
    if (parentWidget()) {
        QRect parentRect = parentWidget()->geometry();
        int x = parentRect.center().x() - width() / 2;
        int y = parentRect.top() + 120;
        move(x, y);
    }

    show();
    raise();
    activateWindow();
    m_searchEdit->setFocus();
}

// ===== 键盘事件 =====
void CommandPalette::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Escape:
            close();
            return;

        case Qt::Key_Up: {
            int current = m_listWidget->currentRow();
            if (current > 0) {
                m_listWidget->setCurrentRow(current - 1);
            } else {
                // 循环到最后一项
                m_listWidget->setCurrentRow(m_listWidget->count() - 1);
            }
            return;
        }

        case Qt::Key_Down: {
            int current = m_listWidget->currentRow();
            if (current < m_listWidget->count() - 1) {
                m_listWidget->setCurrentRow(current + 1);
            } else {
                // 循环到第一项
                m_listWidget->setCurrentRow(0);
            }
            return;
        }

        case Qt::Key_Return:
        case Qt::Key_Enter:
            executeSelected();
            return;
    }

    QDialog::keyPressEvent(event);
}

// ===== 搜索文本变化 =====
void CommandPalette::onSearchTextChanged(const QString &text)
{
    if (!m_listWidget) return;

    m_listWidget->clear();

    for (int i = 0; i < m_commands.size(); ++i) {
        const Command &cmd = m_commands.at(i);

        if (matchesSearch(cmd, text)) {
            QListWidgetItem *item = new QListWidgetItem();

            // 🆕 用 HTML 富文本，让快捷键颜色更淡
            QString iconPart = cmd.icon.isEmpty() ? "" :
                QString("%1&nbsp;&nbsp;").arg(cmd.icon.toHtmlEscaped());

            QString titlePart = cmd.title.toHtmlEscaped();

            QString shortcutPart;
            if (!cmd.shortcut.isEmpty()) {
                shortcutPart = QString(
                    "<span style='color:%1; font-size:13px;'>&nbsp;&nbsp;%2</span>"
                ).arg(m_shortcutColor, cmd.shortcut.toHtmlEscaped());
            }

            // 用 QLabel 支持富文本
            QLabel *label = new QLabel();
            label->setText(iconPart + titlePart + shortcutPart);
            label->setStyleSheet(QString(
                "QLabel { padding: 4px 0px; background: transparent; color: %1; font-size: 15px; }"
            ).arg(m_fgColor));
            label->setTextFormat(Qt::RichText);
            label->setAttribute(Qt::WA_TransparentForMouseEvents);   // 让点击穿透到 item

            m_listWidget->addItem(item);
            m_listWidget->setItemWidget(item, label);

            item->setData(Qt::UserRole, i);
            item->setSizeHint(QSize(0, 42));   // 固定高度
        }
    }

    // 默认选中第一个
    if (m_listWidget->count() > 0) {
        m_listWidget->setCurrentRow(0);
    }
}

// ===== 模糊匹配 =====
bool CommandPalette::matchesSearch(const Command &cmd, const QString &search) const
{
    if (search.isEmpty()) return true;

    QString target = cmd.title.toLower();
    QString query = search.toLower();

    int j = 0;
    for (int i = 0; i < target.length() && j < query.length(); ++i) {
        if (target[i] == query[j]) {
            j++;
        }
    }
    return j == query.length();
}

// ===== 执行选中的命令 =====
void CommandPalette::executeSelected()
{
    QListWidgetItem *item = m_listWidget->currentItem();
    if (!item) return;

    int idx = item->data(Qt::UserRole).toInt();
    if (idx < 0 || idx >= m_commands.size()) return;

    close();

    if (m_commands[idx].action) {
        m_commands[idx].action();
    }
}