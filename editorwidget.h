#ifndef EDITORWIDGET_H
#define EDITORWIDGET_H

#include <QPlainTextEdit>
#include <QVector>

class Highlighter;
class QPaintEvent;
class QResizeEvent;
class QKeyEvent;
class LineNumberArea;
class QWheelEvent;
class LineNumberArea;


struct FoldRegion {
    int startLine;   // 起始行（有 `{` 的行）
    int endLine;     // 结束行（有匹配 `}` 的行）
    bool folded;     // 是否已折叠
};

class EditorWidget : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit EditorWidget(QWidget *parent = nullptr);
    ~EditorWidget() override;

    // 文件相关
    bool loadFile(const QString &fileName);
    bool save();
    bool saveAs(const QString &fileName);

    QString currentFile() const { return m_currentFile; }
    QString userFriendlyName() const;

    bool isModified() const;
    bool maybeSave();

    // 行号栏
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
        
    void zoomIn(int step = 1);// 字体缩放
    void zoomOut(int step = 1);
    void resetZoom();
       
    int cursorPosition() const; //  光标位置访问（用于会话恢复）
    void setCursorPosition(int position);    
    void applySettings();// 应用设置（从 QSettings 读取并应用）
        //  折叠相关
    int foldAreaWidth() const;   // 折叠图标区域宽度
    void foldAreaPaintEvent(QPaintEvent *event);
    void handleFoldAreaClick(int y);   // 处理折叠图标点击
        // 🆕 折叠所有 / 展开所有
    void foldAll();
    void unfoldAll();

    // 🆕 当前已折叠数量
    int foldedCount() const;

    signals:
    void fileInfoChanged();
        // 🆕 折叠数量变化（用于状态栏显示）
    void foldCountChanged(int count);

protected:
    void resizeEvent(QResizeEvent *event) override;
    // 🆕 重写按键事件（用于自动缩进）
    void keyPressEvent(QKeyEvent *event) override;
        //  鼠标滚轮事件（用于 Ctrl+滚轮缩放）
    void wheelEvent(QWheelEvent *event) override;
        // 🆕 重写绘制（用于在折叠行末画提示）
    void paintEvent(QPaintEvent *event) override;
private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();
    void matchBrackets();
    void updateFoldRegions();// 🆕 文本变化时重新检测折叠区域
    void highlightFoldedLines();
private:
    bool saveToFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    void applyHighlighter(const QString &fileName);
    int findMatchingBracket(int position, QChar bracket, bool forward);
    QString currentLineIndent() const;
    QString m_currentFile;
    Highlighter *m_highlighter;
    LineNumberArea *m_lineNumberArea;
    int m_baseFontSize;   
    bool m_showLineNumbers;
    bool m_highlightCurrentLine;
    bool m_matchBracketsEnabled;
    int m_tabSize;
        // 🆕 折叠相关
    QVector<FoldRegion> m_foldRegions;   // 所有可折叠区域
    class FoldArea *m_foldArea;          // 折叠图标区域控件
    // 找出某行开始的折叠区域索引（找不到返回 -1）
    int findFoldRegionByStartLine(int line) const;
};


// LineNumberArea 类保持不变
class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    explicit LineNumberArea(EditorWidget *editor)
        : QWidget(editor), m_editor(editor) {}

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    EditorWidget *m_editor;
};

// ============================================================
// FoldArea：折叠图标区域（在行号栏右边）
// ============================================================
class FoldArea : public QWidget
{
    Q_OBJECT

public:
    explicit FoldArea(EditorWidget *editor)
        : QWidget(editor), m_editor(editor) {}

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    EditorWidget *m_editor;
};

#endif // EDITORWIDGET_H