#ifndef MINDNODE_H
#define MINDNODE_H

#include <QGraphicsItem>
#include <QVector>

class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
class MindEdge;
class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneMouseEvent;
// 节点形状
enum class NodeShape {
    RoundedRect,   // 圆角矩形（默认）
    Ellipse,       // 椭圆
    Rect           // 矩形
};

// 思维导图节点
class MindNode : public QGraphicsItem
{
public:
    explicit MindNode(const QString &text = QString("节点"), MindNode *parent = nullptr);
    ~MindNode() override;

    // ===== 图形项必须实现的两个函数 =====
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    // ===== 类型识别（Qt 图形项建议做）=====
    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    // ===== 属性访问 =====
    QString text() const { return m_text; }
    void setText(const QString &text);

    QColor bgColor() const { return m_bgColor; }
    void setBgColor(const QColor &color);

    QColor borderColor() const { return m_borderColor; }
    void setBorderColor(const QColor &color);

    QColor textColor() const { return m_textColor; }
    void setTextColor(const QColor &color);

    NodeShape nodeShape() const { return m_shape; }
    void setNodeShape(NodeShape shape);

    bool isCenterNode() const { return m_isCenterNode; }
    void setCenterNode(bool center);

    // ===== 树结构 =====
    MindNode *parentNode() const { return m_parentNode; }
    QVector<MindNode*> childNodes() const { return m_childNodes; }

    void addChild(MindNode *child);
    void removeChild(MindNode *child);
            // ===== 连线管理 =====
    void addEdge(MindEdge *edge);
    void removeEdge(MindEdge *edge);
    QVector<MindEdge*> edges() const { return m_edges; }
            
    void deleteWithChildren();// 🆕 递归删除节点（含所有子孙 + 连线）
        // 🆕 唯一 ID（用于保存/加载）
    QString nodeId() const { return m_nodeId; }
    void setNodeId(const QString &id) { m_nodeId = id; }
        // 🆕 折叠/展开
    bool isCollapsed() const { return m_collapsed; }
    void setCollapsed(bool collapsed);
    void toggleCollapsed();

    // 🆕 判断某个位置是否在折叠图标上
    bool isPointOnFoldIcon(const QPointF &localPos) const;
        // 🆕 快速添加子节点/兄弟节点（供快捷键使用）
    MindNode *addChildNode(const QString &text = QString());
    MindNode *addSiblingNode(const QString &text = QString());

    // 🆕 触发编辑（弹出输入框）
    void startEdit();
protected:
    // ===== 悬停事件 =====
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    // ===== 项状态变化 =====
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
            // 🆕 右键菜单
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
            // 🆕 双击编辑
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
        void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
private:
    // 根据文字计算节点大小
    void updateSize();

    QString m_text;               // 节点文字
    QColor m_bgColor;             // 背景色
    QColor m_borderColor;         // 边框色
    QColor m_textColor;           // 文字色
    NodeShape m_shape;            // 形状
    bool m_isCenterNode;          // 是否是中心节点
    bool m_isHovered;             // 是否鼠标悬停

    QSizeF m_size;                // 节点大小（根据文字计算）

    // 树结构
    MindNode *m_parentNode;
    QVector<MindNode*> m_childNodes;
        
    QVector<MindEdge*> m_edges;// 🆕 连接到本节点的所有连线（用于位置变化时通知它们）
            
    QString m_nodeId;// 🆕 节点唯一 ID
        // 🆕 记录拖动开始时的位置（用于生成撤销命令）
    QPointF m_dragStartPos;
    bool m_isDragging;
        // 🆕 折叠状态
    bool m_collapsed;

    // 🆕 递归显示/隐藏子孙节点
    void setChildrenVisible(bool visible);
};

#endif // MINDNODE_H