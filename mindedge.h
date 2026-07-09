#ifndef MINDEDGE_H
#define MINDEDGE_H

#include <QGraphicsItem>

class MindNode;
class QPainter;

// 连接两个节点的曲线
class MindEdge : public QGraphicsItem
{
public:
    MindEdge(MindNode *source, MindNode *dest);
    ~MindEdge() override;

    // QGraphicsItem 必须实现
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    // 类型识别
    enum { Type = UserType + 2 };
    int type() const override { return Type; }

    // 获取两端节点
    MindNode *sourceNode() const { return m_source; }
    MindNode *destNode() const { return m_dest; }

    // 通知连线需要重新计算路径（当节点移动时调用）
    void adjust();

    // 设置线条粗细和颜色
    void setLineWidth(qreal width) { m_lineWidth = width; update(); }
    void setLineColor(const QColor &color) { m_lineColor = color; update(); }

private:
    MindNode *m_source;    // 源节点（一般是父）
    MindNode *m_dest;      // 目标节点（一般是子）

    QPointF m_sourcePoint;   // 源端点
    QPointF m_destPoint;     // 目标端点

    qreal m_lineWidth;
    QColor m_lineColor;
};

#endif // MINDEDGE_H