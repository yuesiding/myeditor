#include "findreplacedialog.h"

#include <QPlainTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTextDocument>
#include <QTextCursor>
#include <QRegularExpression>

// ===== 构造函数 =====
FindReplaceDialog::FindReplaceDialog(QPlainTextEdit *editor, QWidget *parent)
    : QDialog(parent), m_editor(editor)
{
    setWindowTitle(tr("查找和替换"));
    setupUi();

    // 让对话框不阻塞主窗口
    setModal(false);
}

void FindReplaceDialog::setEditor(QPlainTextEdit *editor)
{
    m_editor = editor;
}

// ===== 构建界面 =====
void FindReplaceDialog::setupUi()
{
    // 输入框
    m_findLineEdit = new QLineEdit(this);
    m_replaceLineEdit = new QLineEdit(this);

    // 复选框
    m_caseSensitiveCheckBox = new QCheckBox(tr("区分大小写"), this);
    m_wholeWordCheckBox = new QCheckBox(tr("全词匹配"), this);
    m_regexCheckBox = new QCheckBox(tr("使用正则表达式"), this);

    // 按钮
    m_findNextButton = new QPushButton(tr("查找下一个"), this);
    m_findPreviousButton = new QPushButton(tr("查找上一个"), this);
    m_replaceButton = new QPushButton(tr("替换"), this);
    m_replaceAllButton = new QPushButton(tr("全部替换"), this);

    // 让"查找下一个"成为默认按钮（按回车触发）
    m_findNextButton->setDefault(true);

    // 布局：用网格布局
    QGridLayout *mainLayout = new QGridLayout(this);

    // 第 0 行：查找标签 + 输入框 + 查找下一个按钮
    mainLayout->addWidget(new QLabel(tr("查找:")), 0, 0);
    mainLayout->addWidget(m_findLineEdit, 0, 1);
    mainLayout->addWidget(m_findNextButton, 0, 2);

    // 第 1 行：替换标签 + 输入框 + 替换按钮
    mainLayout->addWidget(new QLabel(tr("替换:")), 1, 0);
    mainLayout->addWidget(m_replaceLineEdit, 1, 1);
    mainLayout->addWidget(m_replaceButton, 1, 2);

    // 第 2 行：查找上一个 + 全部替换
    mainLayout->addWidget(m_findPreviousButton, 2, 1);
    mainLayout->addWidget(m_replaceAllButton, 2, 2);

    // 第 3-5 行：三个复选框
    mainLayout->addWidget(m_caseSensitiveCheckBox, 3, 0, 1, 3);
    mainLayout->addWidget(m_wholeWordCheckBox, 4, 0, 1, 3);
    mainLayout->addWidget(m_regexCheckBox, 5, 0, 1, 3);

    setLayout(mainLayout);
    resize(450, 220);

    // 连接信号
    connect(m_findNextButton, &QPushButton::clicked, this, &FindReplaceDialog::onFindNext);
    connect(m_findPreviousButton, &QPushButton::clicked, this, &FindReplaceDialog::onFindPrevious);
    connect(m_replaceButton, &QPushButton::clicked, this, &FindReplaceDialog::onReplace);
    connect(m_replaceAllButton, &QPushButton::clicked, this, &FindReplaceDialog::onReplaceAll);
}

// ===== 查找下一个 =====
void FindReplaceDialog::onFindNext()
{
    doFind(true);
}

// ===== 查找上一个 =====
void FindReplaceDialog::onFindPrevious()
{
    doFind(false);
}

// ===== 实际执行查找 =====
bool FindReplaceDialog::doFind(bool forward)
{
    if (!m_editor) return false;

    QString searchText = m_findLineEdit->text();
    if (searchText.isEmpty()) return false;

    // 组装查找选项
    QTextDocument::FindFlags flags;
    if (m_caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (m_wholeWordCheckBox->isChecked()) {
        flags |= QTextDocument::FindWholeWords;
    }
    if (!forward) {
        flags |= QTextDocument::FindBackward;
    }

    bool found = false;

    if (m_regexCheckBox->isChecked()) {
        // 使用正则表达式
        QRegularExpression regex(searchText);
        if (!m_caseSensitiveCheckBox->isChecked()) {
            regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
        }
        found = m_editor->find(regex, flags);
    } else {
        // 普通文本
        found = m_editor->find(searchText, flags);
    }

    if (!found) {
        // 找不到，从头/尾再找一次
        QTextCursor cursor = m_editor->textCursor();
        QTextCursor originalCursor = cursor;

        if (forward) {
            cursor.movePosition(QTextCursor::Start);
        } else {
            cursor.movePosition(QTextCursor::End);
        }
        m_editor->setTextCursor(cursor);

        if (m_regexCheckBox->isChecked()) {
            QRegularExpression regex(searchText);
            if (!m_caseSensitiveCheckBox->isChecked()) {
                regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            }
            found = m_editor->find(regex, flags);
        } else {
            found = m_editor->find(searchText, flags);
        }

        if (!found) {
            m_editor->setTextCursor(originalCursor);
            QMessageBox::information(this, tr("查找"),
                                     tr("未找到 \"%1\"").arg(searchText));
        }
    }

    return found;
}

// ===== 替换当前选中 =====
void FindReplaceDialog::onReplace()
{
    if (!m_editor) return;

    QTextCursor cursor = m_editor->textCursor();

    // 如果当前有选中内容，且和搜索文本一致，就替换
    if (cursor.hasSelection()) {
        QString selected = cursor.selectedText();
        QString target = m_findLineEdit->text();

        bool matched = false;
        if (m_regexCheckBox->isChecked()) {
            QRegularExpression regex(target);
            if (!m_caseSensitiveCheckBox->isChecked()) {
                regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            }
            matched = regex.match(selected).hasMatch();
        } else {
            Qt::CaseSensitivity cs = m_caseSensitiveCheckBox->isChecked()
                                      ? Qt::CaseSensitive : Qt::CaseInsensitive;
            matched = (QString::compare(selected, target, cs) == 0);
        }

        if (matched) {
            cursor.insertText(m_replaceLineEdit->text());
        }
    }

    // 然后自动跳到下一个
    onFindNext();
}

// ===== 全部替换 =====
void FindReplaceDialog::onReplaceAll()
{
    if (!m_editor) return;

    QString searchText = m_findLineEdit->text();
    if (searchText.isEmpty()) return;

    // 移到文档开头
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(QTextCursor::Start);
    m_editor->setTextCursor(cursor);

    // 组装选项
    QTextDocument::FindFlags flags;
    if (m_caseSensitiveCheckBox->isChecked()) {
        flags |= QTextDocument::FindCaseSensitively;
    }
    if (m_wholeWordCheckBox->isChecked()) {
        flags |= QTextDocument::FindWholeWords;
    }

    // 循环查找并替换
    int count = 0;
    QString replaceText = m_replaceLineEdit->text();

    // 用 beginEditBlock 让整个替换算一次撤销操作
    m_editor->textCursor().beginEditBlock();

    while (true) {
        bool found;
        if (m_regexCheckBox->isChecked()) {
            QRegularExpression regex(searchText);
            if (!m_caseSensitiveCheckBox->isChecked()) {
                regex.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
            }
            found = m_editor->find(regex, flags);
        } else {
            found = m_editor->find(searchText, flags);
        }

        if (!found) break;

        QTextCursor c = m_editor->textCursor();
        c.insertText(replaceText);
        ++count;
    }

    m_editor->textCursor().endEditBlock();

    QMessageBox::information(this, tr("替换"),
                             tr("共替换了 %1 处").arg(count));
}