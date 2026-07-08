#include "minimapwidget.h"
#include "thememanager.h"

#include <QPlainTextEdit>
#include <QPainter>
#include <QMouseEvent>
#include <QTextBlock>
#include <QTextDocument>
#include <QScrollBar>
#include <QResizeEvent>

MinimapWidget::MinimapWidget(QPlainTextEdit *editor, QWidget *parent)
    : QWidget(parent)
    , m_editor(editor)
    , m_lineHeight(2)   // 每行 2 像素高，非常小
{
    setMouseTracking(true);   // 支持鼠标拖动

    // 监听编辑器变化 → 自动刷新
    if (m_editor) {
        connect(m_editor->document(), &QTextDocument::contentsChanged,
                this, &MinimapWidget::refresh);
        connect(m_editor->verticalScrollBar(), &QScrollBar::valueChanged,
                this, &MinimapWidget::refresh);
    }

    // 监听主题变化
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &MinimapWidget::refresh);
}

QSize MinimapWidget::sizeHint() const
{
    return QSize(80, 0);   // 宽度 80 像素
}

void MinimapWidget::refresh()
{
    update();   // 触发重绘
}

void MinimapWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (!m_editor) return;

    const Theme &theme = ThemeManager::instance().currentTheme();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);   // 关闭抗锯齿更清晰

    // ===== 背景 =====
    painter.fillRect(rect(), theme.editorBackground);

    // ===== 遍历所有行，绘制"缩略条" =====
    QTextDocument *doc = m_editor->document();
    int totalLines = doc->blockCount();
    if (totalLines == 0) return;

    // 计算每行的高度（如果内容太多，自动缩小）
    int availableHeight = height();
    int lineHeight = m_lineHeight;
    if (totalLines * lineHeight > availableHeight) {
        // 内容超出，动态缩小行高
        lineHeight = qMax(1, availableHeight / totalLines);
    }

    // 判断是否暗主题
    bool isDark = theme.editorBackground.lightness() < 128;

    // 每行的颜色（用主题的前景色，稍微淡一点）
    QColor lineColor = theme.editorForeground;
    lineColor.setAlpha(isDark ? 180 : 150);

    // 绘制每一行的"缩略条"
    QTextBlock block = doc->firstBlock();
    int y = 0;
    while (block.isValid() && y < height()) {
        if (block.isVisible()) {
            QString text = block.text();

            // 忽略前导空格，计算实际内容
            QString trimmed = text;
            int leftSpaces = 0;
            for (int i = 0; i < text.length(); ++i) {
                if (text[i] == ' ' || text[i] == '\t') {
                    leftSpaces++;
                } else {
                    break;
                }
            }

            // 换算宽度：字符数按比例映射到 widget 宽度
            // 假设正常代码宽度是 80 字符，映射到 widget 宽度
            int totalWidth = width() - 8;   // 留 8 像素边距
            int codeWidth = text.length() - leftSpaces;

            // 缩略条起点（模拟缩进）
            int startX = 4 + (leftSpaces * totalWidth / 80);
            // 缩略条长度
            int barWidth = qMin(codeWidth * totalWidth / 80, totalWidth - (startX - 4));

            if (barWidth > 0) {
                // 空行不画
                if (!trimmed.trimmed().isEmpty()) {
                    // 根据行首字符判断颜色（简单猜测）
                    QColor c = lineColor;
                    QString stripped = text.trimmed();

                    // 注释用注释色
                    if (stripped.startsWith("//") ||
                        stripped.startsWith("#") ||
                        stripped.startsWith("/*") ||
                        stripped.startsWith("*")) {
                        c = theme.commentColor;
                        c.setAlpha(isDark ? 180 : 150);
                    }
                    // 预处理指令用预处理色
                    else if (stripped.startsWith("#include") ||
                             stripped.startsWith("#define")) {
                        c = theme.preprocessorColor;
                        c.setAlpha(isDark ? 180 : 150);
                    }

                    painter.fillRect(startX, y, barWidth, qMax(1, lineHeight - 1), c);
                }
            }
        }

        block = block.next();
        y += lineHeight;
    }

    // ===== 绘制可视区域框 =====
    QScrollBar *scrollBar = m_editor->verticalScrollBar();
    int scrollMax = scrollBar->maximum();
    int scrollValue = scrollBar->value();
    int scrollRange = scrollMax + scrollBar->pageStep();

        if (scrollRange > 0 && totalLines > 0) {
        // 编辑器可见的行数（大概）
        int visibleLines = m_editor->viewport()->height() /
                           qMax(1, m_editor->fontMetrics().height());

        // 🆕 用滚动条的值来推算当前顶部行
        // scrollValue / scrollMax 的比例，就是"当前位置在整个文档的比例"
        int firstLine = 0;
        if (scrollMax > 0) {
            firstLine = (scrollValue * totalLines) / (scrollMax + scrollBar->pageStep());
        }

        // 在迷你地图上的可视区域位置
        int viewportTop = firstLine * lineHeight;
        int viewportHeight = visibleLines * lineHeight;

        // 绘制半透明高亮框
        QColor overlayColor = isDark ? QColor(255, 255, 255, 30)
                                     : QColor(0, 0, 0, 30);
        painter.fillRect(0, viewportTop, width(), viewportHeight, overlayColor);

        // 绘制框的边框
        QColor borderColor = isDark ? QColor(255, 255, 255, 60)
                                    : QColor(0, 0, 0, 60);
        painter.setPen(borderColor);
        painter.drawRect(0, viewportTop, width() - 1, viewportHeight - 1);
    }
}

void MinimapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        scrollEditorToPosition(event->pos().y());
    }
}

void MinimapWidget::mouseMoveEvent(QMouseEvent *event)
{
    // 按住鼠标拖动时也滚动
    if (event->buttons() & Qt::LeftButton) {
        scrollEditorToPosition(event->pos().y());
    }
}

void MinimapWidget::scrollEditorToPosition(int y)
{
    if (!m_editor) return;

    QTextDocument *doc = m_editor->document();
    int totalLines = doc->blockCount();
    if (totalLines == 0) return;

    int availableHeight = height();
    int lineHeight = m_lineHeight;
    if (totalLines * lineHeight > availableHeight) {
        lineHeight = qMax(1, availableHeight / totalLines);
    }

    // 换算：y 像素对应哪一行
    int targetLine = y / lineHeight;
    targetLine = qBound(0, targetLine, totalLines - 1);

    // 跳到目标行（作为顶部）
    QTextCursor cursor(doc->findBlockByNumber(targetLine));
    m_editor->setTextCursor(cursor);
    m_editor->centerCursor();   // 让光标居中显示
}