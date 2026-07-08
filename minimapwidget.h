#ifndef MINIMAPWIDGET_H
#define MINIMAPWIDGET_H

#include <QWidget>

class QPlainTextEdit;
class QPaintEvent;
class QMouseEvent;
class QResizeEvent;

class MinimapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MinimapWidget(QPlainTextEdit *editor, QWidget *parent = nullptr);

    // 建议宽度
    QSize sizeHint() const override;

public slots:
    // 编辑器内容或滚动变化时调用
    void refresh();

protected:
    // 绘制迷你地图
    void paintEvent(QPaintEvent *event) override;

    // 鼠标点击/拖动 → 编辑器跳转
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    // 根据鼠标 y 坐标，跳转到对应行
    void scrollEditorToPosition(int y);

    QPlainTextEdit *m_editor;

    // 每行在迷你地图里的高度（像素）
    int m_lineHeight;
};

#endif // MINIMAPWIDGET_H