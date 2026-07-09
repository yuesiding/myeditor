#include "mindedge.h"
#include "mindnode.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionGraphicsItem>

MindEdge::MindEdge(MindNode *source, MindNode *dest)
    : QGraphicsItem()
    , m_source(source)
    , m_dest(dest)
    , m_lineWidth(2.0)
    , m_lineColor(QColor("#7F8C8D"))
{
    // 连线放在节点下面
    setZValue(-1);

    // 计算初始位置
    adjust();
}

MindEdge::~MindEdge()
{
}

// ===== 更新端点位置 =====
void MindEdge::adjust()
{
    if (!m_source || !m_dest) return;

    // 端点就是两个节点的中心
    QPointF newSource = m_source->pos();
    QPointF newDest = m_dest->pos();

    if (m_sourcePoint != newSource || m_destPoint != newDest) {
        prepareGeometryChange();
        m_sourcePoint = newSource;
        m_destPoint = newDest;
    }
}

// ===== 边界矩形 =====
QRectF MindEdge::boundingRect() const
{
    // 包围两个端点的矩形（加点余量）
    qreal padding = m_lineWidth + 10;
    return QRectF(m_sourcePoint, m_destPoint)
                .normalized()
                .adjusted(-padding, -padding, padding, padding);
}

// ===== 绘制曲线 =====
void MindEdge::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_source || !m_dest) return;

    painter->setRenderHint(QPainter::Antialiasing);

    // 画笔
    QPen pen(m_lineColor, m_lineWidth, Qt::SolidLine,
             Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    // 构造贝塞尔曲线
    QPainterPath path;
    path.moveTo(m_sourcePoint);

    // 计算控制点，让曲线呈"S 形"或"C 形"，更美观
    qreal dx = m_destPoint.x() - m_sourcePoint.x();
    // 控制点在起点和终点之间，水平方向偏移
    QPointF ctrl1(m_sourcePoint.x() + dx * 0.5, m_sourcePoint.y());
    QPointF ctrl2(m_destPoint.x() - dx * 0.5, m_destPoint.y());

    path.cubicTo(ctrl1, ctrl2, m_destPoint);

    painter->drawPath(path);
}