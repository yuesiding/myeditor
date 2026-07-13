#include "editorwidget.h"
#include "cpphighlighter.h"
#include "thememanager.h"
#include "generichighlighter.h"
#include "syntaxmanager.h"
#include "mainwindow.h"
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QFont>
#include <QPainter>
#include <QTextBlock>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QSettings>
#include <QMouseEvent>
#include <QDebug>
#include <QContextMenuEvent>
#include <QMenu>


// ============================================================
// LineNumberArea 实现
// ============================================================
QSize LineNumberArea::sizeHint() const
{
    return QSize(m_editor->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event)
{
    m_editor->lineNumberAreaPaintEvent(event);
}


EditorWidget::EditorWidget(QWidget *parent)
    : QPlainTextEdit(parent), 
      m_highlighter(nullptr), 
      m_baseFontSize(11),
      m_showLineNumbers(true),        
      m_highlightCurrentLine(true),      
      m_matchBracketsEnabled(true),      
      m_tabSize(4)
{
    QFont font("Consolas", m_baseFontSize);
    font.setStyleHint(QFont::Monospace);
    setFont(font);

    m_lineNumberArea = new LineNumberArea(this);
    m_foldArea = new FoldArea(this);   
    connect(this, &EditorWidget::blockCountChanged,
            this, &EditorWidget::updateLineNumberAreaWidth);
    connect(this, &EditorWidget::updateRequest,
            this, &EditorWidget::updateLineNumberArea);
    connect(this, &EditorWidget::cursorPositionChanged,
            this, &EditorWidget::highlightCurrentLine);
    connect(this, &EditorWidget::cursorPositionChanged,
            this, &EditorWidget::highlightFoldedLines);
    connect(this, &EditorWidget::cursorPositionChanged,
            this, &EditorWidget::matchBrackets);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();

    connect(document(), &QTextDocument::modificationChanged,
            this, &EditorWidget::fileInfoChanged);
        // 🆕 文本变化时重新检测折叠区域
    connect(this, &EditorWidget::blockCountChanged,
            this, &EditorWidget::updateFoldRegions);
    connect(document(), &QTextDocument::contentsChanged,
            this, &EditorWidget::updateFoldRegions);    
    // 🆕 应用主题
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, [this]() {
                const Theme &theme = ThemeManager::instance().currentTheme();
                QPalette pal = palette();
                pal.setColor(QPalette::Base, theme.editorBackground);
                pal.setColor(QPalette::Text, theme.editorForeground);
                setPalette(pal);
                m_lineNumberArea->update();
                highlightCurrentLine();
            });

    const Theme &theme = ThemeManager::instance().currentTheme();
    QPalette pal = palette();
    pal.setColor(QPalette::Base, theme.editorBackground);
    pal.setColor(QPalette::Text, theme.editorForeground);
    setPalette(pal);
    applySettings();
}

EditorWidget::~EditorWidget()
{
}

// ===== 行号栏 =====
int EditorWidget::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }
    int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

void EditorWidget::updateLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth() + foldAreaWidth(), 0, 0, 0);
}

void EditorWidget::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
        m_foldArea->scroll(0, dy);   // 🆕
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
        m_foldArea->update(0, rect.y(), m_foldArea->width(), rect.height());   // 🆕
    }
    if (rect.contains(viewport()->rect())) {
        updateLineNumberAreaWidth(0);
    }
}

void EditorWidget::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();

    // 行号栏
    m_lineNumberArea->setGeometry(
        QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height())
    );

    // 折叠区域
    m_foldArea->setGeometry(
        QRect(cr.left() + lineNumberAreaWidth(), cr.top(),
              foldAreaWidth(), cr.height())
    );

}

void EditorWidget::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    const Theme &theme = ThemeManager::instance().currentTheme();

    QPainter painter(m_lineNumberArea);
        // 背景（比编辑区稍暗，形成对比）
    QColor bgColor = theme.editorBackground;
    bool isDarkTheme = theme.editorBackground.lightness() < 128;
    if (isDarkTheme) {
        bgColor = bgColor.darker(120);   // 暗主题：更暗一点
    } else {
        bgColor = bgColor.darker(105);   // 亮主题：稍暗
    }
    painter.fillRect(rect(), bgColor);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(theme.lineNumberForeground);
            painter.drawText(0, top,
                             m_lineNumberArea->width() - 5,
                             fontMetrics().height(),
                             Qt::AlignRight, number);
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void EditorWidget::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly() && m_highlightCurrentLine) {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = ThemeManager::instance().currentTheme().currentLineHighlight;
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}
// ===== 🆕 括号匹配高亮 =====
void EditorWidget::matchBrackets()
{
     if (!m_matchBracketsEnabled) {
        return;
    }
    // 保留当前行高亮
    QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();

    QTextCursor cursor = textCursor();
    int pos = cursor.position();
    QString text = document()->toPlainText();

    if (text.isEmpty()) {
        setExtraSelections(extraSelections);
        return;
    }

    // 检查光标左右两个字符
    QChar leftChar = (pos > 0) ? text.at(pos - 1) : QChar();
    QChar rightChar = (pos < text.length()) ? text.at(pos) : QChar();

    int bracketPos = -1;
    int matchPos = -1;

    // 判断右边是不是左括号
    if (rightChar == '(' || rightChar == '{' || rightChar == '[') {
        bracketPos = pos;
        matchPos = findMatchingBracket(pos, rightChar, true);
    }
    // 判断左边是不是右括号
    else if (leftChar == ')' || leftChar == '}' || leftChar == ']') {
        bracketPos = pos - 1;
        matchPos = findMatchingBracket(pos - 1, leftChar, false);
    }

    // 如果找到了匹配，就高亮这两个位置
    if (bracketPos >= 0 && matchPos >= 0) {
        QTextCharFormat format;
        format.setBackground(ThemeManager::instance().currentTheme().bracketMatchColor);
        format.setFontWeight(QFont::Bold);

        // 高亮起始括号
        QTextEdit::ExtraSelection sel1;
        sel1.format = format;
        sel1.cursor = QTextCursor(document());
        sel1.cursor.setPosition(bracketPos);
        sel1.cursor.setPosition(bracketPos + 1, QTextCursor::KeepAnchor);
        extraSelections.append(sel1);

        // 高亮匹配括号
        QTextEdit::ExtraSelection sel2;
        sel2.format = format;
        sel2.cursor = QTextCursor(document());
        sel2.cursor.setPosition(matchPos);
        sel2.cursor.setPosition(matchPos + 1, QTextCursor::KeepAnchor);
        extraSelections.append(sel2);
    }

    setExtraSelections(extraSelections);
}

// ===== 🆕 找匹配的括号 =====
int EditorWidget::findMatchingBracket(int position, QChar bracket, bool forward)
{
    QString text = document()->toPlainText();
    QChar target;

    // 确定要找的目标括号
    if (bracket == '(') target = ')';
    else if (bracket == ')') target = '(';
    else if (bracket == '{') target = '}';
    else if (bracket == '}') target = '{';
    else if (bracket == '[') target = ']';
    else if (bracket == ']') target = '[';
    else return -1;

    int depth = 1;
    if (forward) {
        // 向后找
        for (int i = position + 1; i < text.length(); ++i) {
            QChar c = text.at(i);
            if (c == bracket) depth++;
            else if (c == target) {
                depth--;
                if (depth == 0) return i;
            }
        }
    } else {
        // 向前找
        for (int i = position - 1; i >= 0; --i) {
            QChar c = text.at(i);
            if (c == bracket) depth++;
            else if (c == target) {
                depth--;
                if (depth == 0) return i;
            }
        }
    }
    return -1;
}

// ===== 🆕 获取当前行的缩进 =====
QString EditorWidget::currentLineIndent() const
{
    QTextCursor cursor = textCursor();
    QString lineText = cursor.block().text();

    QString indent;
    for (int i = 0; i < lineText.length(); ++i) {
        QChar c = lineText.at(i);
        if (c == ' ' || c == '\t') {
            indent += c;
        } else {
            break;
        }
    }
    return indent;
}

// ===== 🆕 重写按键事件（用于自动缩进）=====
void EditorWidget::keyPressEvent(QKeyEvent *event)
{
    // 处理回车键
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        QTextCursor cursor = textCursor();
        QString lineText = cursor.block().text();
        QString indent = currentLineIndent();

        // 检查上一行是否以 { 结尾（需要多缩进一级）
        QString trimmed = lineText.trimmed();
        bool extraIndent = trimmed.endsWith('{') || trimmed.endsWith('(') || trimmed.endsWith('[');

        // 先执行默认的回车（换行）
        QPlainTextEdit::keyPressEvent(event);

        // 然后插入缩进
        QTextCursor newCursor = textCursor();
        newCursor.insertText(indent);
        if (extraIndent) {
            newCursor.insertText("    ");  // 4 个空格
        }
        return;
    }

    // 其他按键交给父类处理
    QPlainTextEdit::keyPressEvent(event);
}


// ============================================================
// 文件操作代码（保持不变）
// ============================================================
bool EditorWidget::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("警告"),
                             tr("无法打开文件：%1").arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    setPlainText(in.readAll());
    file.close();

    setCurrentFile(fileName);
    applyHighlighter(fileName);
    return true;
}

bool EditorWidget::save()
{
    if (m_currentFile.isEmpty()) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("保存文件"));
        if (fileName.isEmpty()) {
            return false;
        }
        return saveToFile(fileName);
    } else {
        return saveToFile(m_currentFile);
    }
}

bool EditorWidget::saveAs(const QString &fileName)
{
    return saveToFile(fileName);
}

bool EditorWidget::saveToFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("警告"),
                             tr("无法保存文件：%1").arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out << toPlainText();
    file.close();

    setCurrentFile(fileName);
    applyHighlighter(fileName);
    return true;
}

void EditorWidget::setCurrentFile(const QString &fileName)
{
    m_currentFile = fileName;
    document()->setModified(false);
    emit fileInfoChanged();
}

QString EditorWidget::userFriendlyName() const
{
    if (m_currentFile.isEmpty()) {
        return tr("未命名");
    }
    return QFileInfo(m_currentFile).fileName();
}

bool EditorWidget::isModified() const
{
    return document()->isModified();
}

bool EditorWidget::maybeSave()
{
    if (!isModified()) {
        return true;
    }

    QMessageBox::StandardButton ret = QMessageBox::warning(
        this,
        tr("提示"),
        tr("文件 \"%1\" 已被修改，是否保存？").arg(userFriendlyName()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
    );

    if (ret == QMessageBox::Save) {
        return save();
    } else if (ret == QMessageBox::Cancel) {
        return false;
    }
    return true;
}

void EditorWidget::applyHighlighter(const QString &fileName)
{
    if (m_highlighter) {
        delete m_highlighter;
        m_highlighter = nullptr;
    }

    QString suffix = QFileInfo(fileName).suffix().toLower();

    // 🆕 从 SyntaxManager 查找对应语言
    SyntaxDefinition def;
    if (SyntaxManager::instance().findByExtension(suffix, def)) {
        m_highlighter = new GenericHighlighter(def, document());
        m_highlighter->rehighlight();
        qDebug() << "应用高亮器:" << def.name() << "for" << fileName;
    } else {
        qDebug() << "未找到匹配的语法定义 for" << suffix;
    }
}

// ============================================================
// 🆕 字体缩放
// ============================================================

// 放大
void EditorWidget::zoomIn(int step)
{
    QFont f = font();
    int newSize = f.pointSize() + step;
    if (newSize > 40) newSize = 40;   // 上限
    f.setPointSize(newSize);
    setFont(f);
}

// 缩小
void EditorWidget::zoomOut(int step)
{
    QFont f = font();
    int newSize = f.pointSize() - step;
    if (newSize < 6) newSize = 6;   // 下限
    f.setPointSize(newSize);
    setFont(f);
}

// 重置为默认
void EditorWidget::resetZoom()
{
    QFont f = font();
    f.setPointSize(m_baseFontSize);
    setFont(f);
}

// ===== Ctrl+滚轮缩放 =====
void EditorWidget::wheelEvent(QWheelEvent *event)
{
    // 检查是否按住 Ctrl
    if (event->modifiers() & Qt::ControlModifier) {
        // Ctrl+滚轮 → 缩放
        int delta = event->angleDelta().y();
        if (delta > 0) {
            zoomIn();
        } else if (delta < 0) {
            zoomOut();
        }
        event->accept();   // 事件已处理
    } else {
        // 普通滚轮 → 交给父类处理（滚动内容）
        QPlainTextEdit::wheelEvent(event);
    }
}

// ============================================================
//  光标位置访问
// ============================================================
int EditorWidget::cursorPosition() const
{
    return textCursor().position();
}

void EditorWidget::setCursorPosition(int position)
{
    QTextCursor cursor = textCursor();
    cursor.setPosition(position);
    setTextCursor(cursor);
    ensureCursorVisible();   // 滚动到光标位置
}

// ============================================================
// 🆕 应用设置
// ============================================================
void EditorWidget::applySettings()
{
    QSettings settings("MyCompany", "CodeEditor");

    // 字体
    QString fontName = settings.value("editor/fontName", "Consolas").toString();
    int fontSize = settings.value("editor/fontSize", 11).toInt();
    QFont f(fontName, fontSize);
    f.setStyleHint(QFont::Monospace);
    setFont(f);
    m_baseFontSize = fontSize;

    // 显示行号
    m_showLineNumbers = settings.value("editor/showLineNumbers", true).toBool();
    if (m_showLineNumbers) {
        m_lineNumberArea->show();
        updateLineNumberAreaWidth(0);
    } else {
        m_lineNumberArea->hide();
        setViewportMargins(0, 0, 0, 0);
    }

    // 高亮当前行
    m_highlightCurrentLine = settings.value("editor/highlightCurrentLine", true).toBool();
    highlightCurrentLine();   // 重新应用（如果关闭会清空高亮）

    // 括号匹配
    m_matchBracketsEnabled = settings.value("editor/matchBrackets", true).toBool();
    matchBrackets();

    // Tab 大小
    m_tabSize = settings.value("editor/tabSize", 4).toInt();
    setTabStopDistance(fontMetrics().horizontalAdvance(' ') * m_tabSize);
}

// ============================================================
// 🆕 FoldArea 实现
// ============================================================
QSize FoldArea::sizeHint() const
{
    return QSize(m_editor->foldAreaWidth(), 0);
}

void FoldArea::paintEvent(QPaintEvent *event)
{
    m_editor->foldAreaPaintEvent(event);
}

void FoldArea::mousePressEvent(QMouseEvent *event)
{
    m_editor->handleFoldAreaClick(event->pos().y());
}


// ============================================================
// 🆕 折叠相关：EditorWidget 里的实现
// ============================================================

// 折叠区域的宽度（固定 16 像素）
int EditorWidget::foldAreaWidth() const
{
    return 16;
}

// 更新折叠区域（扫描代码，找出所有 { 位置）
// 更新折叠区域（扫描代码，找出所有 { 位置）
void EditorWidget::updateFoldRegions()
{
    // 🆕 先记住之前已折叠的区域（用起始行标识）
    QVector<int> previouslyFolded;
    for (const FoldRegion &r : m_foldRegions) {
        if (r.folded) {
            previouslyFolded.append(r.startLine);
        }
    }

    m_foldRegions.clear();

    QString text = document()->toPlainText();

    struct BraceInfo {
        int line;
        int pos;
    };
    QVector<BraceInfo> stack;

    int currentLine = 0;
    for (int i = 0; i < text.length(); ++i) {
        QChar c = text.at(i);

        if (c == '\n') {
            currentLine++;
        } else if (c == '{') {
            stack.append({currentLine, i});
        } else if (c == '}') {
            if (!stack.isEmpty()) {
                BraceInfo info = stack.last();
                stack.removeLast();

                if (currentLine > info.line) {
                    FoldRegion region;
                    region.startLine = info.line;
                    region.endLine = currentLine;
                    // 🆕 如果之前折叠过，保持折叠
                    region.folded = previouslyFolded.contains(info.line);
                    m_foldRegions.append(region);
                }
            }
        }
    }

    m_foldArea->update();
    emit foldCountChanged(foldedCount());
}

// 查找某行开始的折叠区域
int EditorWidget::findFoldRegionByStartLine(int line) const
{
    for (int i = 0; i < m_foldRegions.size(); ++i) {
        if (m_foldRegions.at(i).startLine == line) {
            return i;
        }
    }
    return -1;
}

// 绘制折叠图标
void EditorWidget::foldAreaPaintEvent(QPaintEvent *event)
{
    const Theme &theme = ThemeManager::instance().currentTheme();

    QPainter painter(m_foldArea);
    painter.fillRect(event->rect(), theme.lineNumberBackground);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    // 🆕 使用更醒目的字体
    QFont iconFont = painter.font();
    iconFont.setPointSize(iconFont.pointSize() + 2);
    iconFont.setBold(true);
    painter.setFont(iconFont);

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            int idx = findFoldRegionByStartLine(blockNumber);
            if (idx >= 0) {
                int x = 2;

                if (m_foldRegions.at(idx).folded) {
                    // 🆕 已折叠：用醒目的橙色
                    painter.setPen(QColor("#3498DB"));
                    painter.drawText(x, top, m_foldArea->width() - 4,
                                     fontMetrics().height(),
                                     Qt::AlignVCenter | Qt::AlignHCenter, "▶");
                } else {
                    // 🆕 已展开：用主题的行号色
                    painter.setPen(theme.lineNumberForeground);
                    painter.drawText(x, top, m_foldArea->width() - 4,
                                     fontMetrics().height(),
                                     Qt::AlignVCenter | Qt::AlignHCenter, "▼");
                }
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

// 处理折叠区域的点击
void EditorWidget::handleFoldAreaClick(int y)
{
    // 计算点击的是哪一行
    QTextBlock block = firstVisibleBlock();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    int blockNumber = block.blockNumber();

    while (block.isValid()) {
        if (block.isVisible() && y >= top && y < bottom) {
            // 找到点击的行
            int idx = findFoldRegionByStartLine(blockNumber);
            if (idx >= 0) {
                // 切换折叠状态
                m_foldRegions[idx].folded = !m_foldRegions[idx].folded;

                // 🆕 真正执行折叠/展开
                bool shouldFold = m_foldRegions[idx].folded;
                int startLine = m_foldRegions[idx].startLine;
                int endLine = m_foldRegions[idx].endLine;

                // 隐藏/显示 [startLine+1, endLine] 之间的行
                // 注意：startLine 本身要保持可见（那行有 { 和图标）
                for (int line = startLine + 1; line <= endLine; ++line) {
                    QTextBlock b = document()->findBlockByNumber(line);
                    if (b.isValid()) {
                        b.setVisible(!shouldFold);
                    }
                }

                // 如果光标在折叠区域内，把它移出去
                if (shouldFold) {
                    QTextCursor cursor = textCursor();
                    int cursorLine = cursor.blockNumber();
                    if (cursorLine > startLine && cursorLine <= endLine) {
                        // 移到起始行
                        QTextBlock startBlock = document()->findBlockByNumber(startLine);
                        cursor.setPosition(startBlock.position());
                        setTextCursor(cursor);
                    }
                }

                // 通知文档：内容布局需要重算
                document()->markContentsDirty(
                    document()->findBlockByNumber(startLine).position(),
                    document()->findBlockByNumber(endLine).position()
                );

                // 触发全面重绘
                viewport()->update();
                m_lineNumberArea->update();
                m_foldArea->update();
                               
                highlightFoldedLines(); // 🆕 更新折叠行的高亮
                emit foldCountChanged(foldedCount());// 🆕 通知状态栏
                qDebug() << "折叠区域" << startLine << "->" << endLine
                         << "，状态：" << (shouldFold ? "已折叠" : "已展开");
            }
            break;
        }
        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

// ============================================================
// 🆕 高亮所有已折叠的行
// ============================================================
void EditorWidget::highlightFoldedLines()
{
    // 先保留已有的 extraSelections（当前行高亮、括号匹配等）
    QList<QTextEdit::ExtraSelection> extraSelections = this->extraSelections();

    // 移除之前的折叠高亮（避免叠加）
    QList<QTextEdit::ExtraSelection> filtered;
    for (const QTextEdit::ExtraSelection &sel : extraSelections) {
        // 用 property 标识哪些是折叠高亮
        if (!sel.format.hasProperty(QTextFormat::UserProperty + 100)) {
            filtered.append(sel);
        }
    }

    // 为每个已折叠的区域添加高亮
    for (const FoldRegion &region : m_foldRegions) {
        if (!region.folded) continue;

        QTextEdit::ExtraSelection sel;

        // 折叠行的背景色（边框）
        QColor bgColor("#E8F5E9");   // 淡薄荷绿
        sel.format.setBackground(bgColor);
        sel.format.setProperty(QTextFormat::FullWidthSelection, true);

        // 标识这是"折叠高亮"（用于后续区分）
        sel.format.setProperty(QTextFormat::UserProperty + 100, true);

        // 设置光标到折叠起始行
        QTextBlock block = document()->findBlockByNumber(region.startLine);
        if (block.isValid()) {
            QTextCursor cursor(block);
            sel.cursor = cursor;
            sel.cursor.clearSelection();
            filtered.append(sel);
        }
    }

    setExtraSelections(filtered);
}

// ============================================================
// 🆕 重写 paintEvent：在折叠行末画提示
// ============================================================
void EditorWidget::paintEvent(QPaintEvent *event)
{
    // 先让父类正常绘制代码
    QPlainTextEdit::paintEvent(event);

    // 再在折叠行末画提示
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    // 提示文字用淡灰色 + 斜体 + 小一号字体
    QFont hintFont = font();
    hintFont.setItalic(true);
    hintFont.setPointSize(hintFont.pointSize() - 1);
    painter.setFont(hintFont);
    painter.setPen(QColor("#888888"));

    QFontMetrics fm(hintFont);

    // 遍历所有折叠区域，找已折叠的
    for (const FoldRegion &region : m_foldRegions) {
        if (!region.folded) continue;

        // 找到起始行的位置
        QTextBlock block = document()->findBlockByNumber(region.startLine);
        if (!block.isValid() || !block.isVisible()) continue;

        // 计算这一行的位置
        QRectF blockRect = blockBoundingGeometry(block).translated(contentOffset());

        // 该行的文本
        QString lineText = block.text();

        // 计算文本结束的 x 位置
        int textEndX = fontMetrics().horizontalAdvance(lineText) + 4;

        // 折叠提示文字
        int hiddenLines = region.endLine - region.startLine;
        QString hint = QString("  ⋯ %1 行已隐藏 ⋯").arg(hiddenLines);

        // 绘制在行末
        painter.drawText(
            QPointF(textEndX, blockRect.top() + fm.ascent() + 2),
            hint
        );
    }
}

// ============================================================
// 🆕 折叠数量 + 一键操作
// ============================================================
int EditorWidget::foldedCount() const
{
    int count = 0;
    for (const FoldRegion &r : m_foldRegions) {
        if (r.folded) count++;
    }
    return count;
}

void EditorWidget::foldAll()
{
    for (FoldRegion &region : m_foldRegions) {
        if (region.folded) continue;   // 已折叠的跳过

        region.folded = true;
        for (int line = region.startLine + 1; line <= region.endLine; ++line) {
            QTextBlock b = document()->findBlockByNumber(line);
            if (b.isValid()) {
                b.setVisible(false);
            }
        }
    }

    // 重算布局
    document()->markContentsDirty(0, document()->characterCount());
    viewport()->update();
    m_lineNumberArea->update();
    m_foldArea->update();
    highlightFoldedLines();

    emit foldCountChanged(foldedCount());
}

void EditorWidget::unfoldAll()
{
    for (FoldRegion &region : m_foldRegions) {
        if (!region.folded) continue;

        region.folded = false;
        for (int line = region.startLine + 1; line <= region.endLine; ++line) {
            QTextBlock b = document()->findBlockByNumber(line);
            if (b.isValid()) {
                b.setVisible(true);
            }
        }
    }

    document()->markContentsDirty(0, document()->characterCount());
    viewport()->update();
    m_lineNumberArea->update();
    m_foldArea->update();
    highlightFoldedLines();

    emit foldCountChanged(foldedCount());
}


// ============================================================
// 🆕 自定义右键菜单（含 AI 功能）
// ============================================================
void EditorWidget::contextMenuEvent(QContextMenuEvent *event)
{
    // 用 Qt 提供的标准菜单作为基础（含剪切、复制、粘贴等）
    QMenu *menu = createStandardContextMenu();

    // 获取选中的代码
    QString selectedCode = textCursor().selectedText();

    // 如果有选中代码，就加 AI 相关菜单
    if (!selectedCode.isEmpty()) {
        // Qt 的 selectedText 会用 \u2029 代替换行符，要替换回来
        selectedCode.replace(QChar(0x2029), '\n');

        menu->addSeparator();

        // 找到父窗口 MainWindow
        MainWindow *mainWin = nullptr;
        QWidget *p = parentWidget();
        while (p) {
            mainWin = qobject_cast<MainWindow *>(p);
            if (mainWin) break;
            p = p->parentWidget();
        }

        if (mainWin) {
            // 🤖 AI 解释
            QAction *explainAct = menu->addAction(tr("🤖 AI 解释这段代码"));
            connect(explainAct, &QAction::triggered, [mainWin, selectedCode]() {
                mainWin->askAiAboutCode(
                    tr("请用中文简洁地解释下面这段代码的功能和实现思路："),
                    selectedCode);
            });

            // 🚀 AI 优化
            QAction *optimizeAct = menu->addAction(tr("🚀 AI 优化这段代码"));
            connect(optimizeAct, &QAction::triggered, [mainWin, selectedCode]() {
                mainWin->askAiAboutCode(
                    tr("请优化下面这段代码，提高性能、可读性或者简洁性。"
                       "请给出优化后的完整代码，并说明改动了什么："),
                    selectedCode);
            });

            // 🐛 AI 找 Bug
            QAction *bugAct = menu->addAction(tr("🐛 AI 找 Bug"));
            connect(bugAct, &QAction::triggered, [mainWin, selectedCode]() {
                mainWin->askAiAboutCode(
                    tr("请仔细检查下面这段代码，找出可能的 bug、"
                       "潜在问题、边界情况处理不当的地方："),
                    selectedCode);
            });

            // 📝 AI 添加注释
            QAction *commentAct = menu->addAction(tr("📝 AI 添加详细注释"));
            connect(commentAct, &QAction::triggered, [mainWin, selectedCode]() {
                mainWin->askAiAboutCode(
                    tr("请给下面这段代码添加详细的中文注释，帮助理解每一部分的作用。"
                       "请返回添加注释后的完整代码："),
                    selectedCode);
            });

            // 🌐 AI 翻译成其他语言
            QAction *translateAct = menu->addAction(tr("🌐 AI 转换为 Python"));
            connect(translateAct, &QAction::triggered, [mainWin, selectedCode]() {
                mainWin->askAiAboutCode(
                    tr("请把下面这段代码用 Python 重新实现，"
                       "保持相同的功能，并添加必要的注释："),
                    selectedCode);
            });
        }
    } else {
        // 没有选中代码时，加"问 AI 一个问题"
        menu->addSeparator();
        MainWindow *mainWin = nullptr;
        QWidget *p = parentWidget();
        while (p) {
            mainWin = qobject_cast<MainWindow *>(p);
            if (mainWin) break;
            p = p->parentWidget();
        }
        if (mainWin) {
            QAction *askAct = menu->addAction(tr("🤖 打开 AI 助手"));
            connect(askAct, &QAction::triggered, [mainWin]() {
                // 只显示 AI 助手，不问问题
                mainWin->askAiAboutCode(QString(), QString());
            });
        }
    }

    menu->exec(event->globalPos());
    delete menu;
}