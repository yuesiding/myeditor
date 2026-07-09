#include "mindnode.h"
#include "mindedge.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QFontMetrics>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsScene>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
#include <QInputDialog>
#include <QColorDialog>
#include <QMessageBox>

MindNode::MindNode(const QString &text, MindNode *parent)
    : QGraphicsItem()
    , m_text(text)
    , m_bgColor(QColor("#4A90E2"))       // 默认蓝色
    , m_borderColor(QColor("#2C5F8D"))
    , m_textColor(QColor("#FFFFFF"))
    , m_shape(NodeShape::RoundedRect)
    , m_isCenterNode(false)
    , m_isHovered(false)
    , m_parentNode(parent)
{
    // 让节点能被选中
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    // 让节点能被拖动
    setFlag(QGraphicsItem::ItemIsMovable, true);

    // 位置变化时发送通知（让连线跟随更新）
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    // 启用鼠标悬停事件
    setAcceptHoverEvents(true);

    // 光标样式
    setCursor(Qt::PointingHandCursor);

    // 根据文字计算大小
    updateSize();

    // 如果有父节点，加到父节点的子列表
    if (parent) {
        parent->addChild(this);
    }
}

MindNode::~MindNode()
{
    // 从父节点的子列表移除
    if (m_parentNode) {
        m_parentNode->removeChild(this);
    }
}

// ===== 计算节点大小（根据文字）=====
void MindNode::updateSize()
{
    // 用当前字体测量文字宽度
    QFont font;
    if (m_isCenterNode) {
        font.setPointSize(14);
        font.setBold(true);
    } else {
        font.setPointSize(11);
    }

    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(m_text);

    // 节点大小 = 文字大小 + 内边距
    qreal padding = m_isCenterNode ? 24 : 16;
    qreal width = qMax(qreal(80.0), textRect.width() + padding * 2);
    qreal height = qMax(qreal(40.0), textRect.height() + padding);

    m_size = QSizeF(width, height);

    // 通知场景需要重绘
    prepareGeometryChange();
}

// ===== 边界矩形（Qt 要求）=====
QRectF MindNode::boundingRect() const
{
    // 稍微大一点，留出阴影空间
    qreal shadow = 4;
    return QRectF(-m_size.width() / 2 - shadow,
                  -m_size.height() / 2 - shadow,
                  m_size.width() + shadow * 2,
                  m_size.height() + shadow * 2);
}

// ===== 绘制节点 =====
void MindNode::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    // 节点矩形（以 (0,0) 为中心）
    QRectF rect(-m_size.width() / 2,
                -m_size.height() / 2,
                m_size.width(),
                m_size.height());

    // ===== 1. 绘制阴影（选中/悬停时更明显）=====
    if (isSelected() || m_isHovered) {
        QRectF shadowRect = rect.translated(2, 2);
        QColor shadow = m_borderColor;
        shadow.setAlpha(60);
        painter->setPen(Qt::NoPen);
        painter->setBrush(shadow);

        if (m_shape == NodeShape::Ellipse) {
            painter->drawEllipse(shadowRect);
        } else {
            qreal radius = m_shape == NodeShape::RoundedRect ? 12 : 0;
            painter->drawRoundedRect(shadowRect, radius, radius);
        }
    }

    // ===== 2. 绘制背景 =====
    QColor bg = m_bgColor;
    if (m_isHovered && !isSelected()) {
        // 悬停时略微变亮
        bg = bg.lighter(115);
    }
    painter->setBrush(bg);

    // ===== 3. 绘制边框 =====
    QPen borderPen;
    if (isSelected()) {
        // 选中：粗边框 + 高亮色
        borderPen.setColor(QColor("#FFB800"));   // 金黄色
        borderPen.setWidth(3);
    } else {
        borderPen.setColor(m_borderColor);
        borderPen.setWidth(m_isCenterNode ? 2 : 1);
    }
    painter->setPen(borderPen);

    // ===== 4. 根据形状绘制 =====
    if (m_shape == NodeShape::Ellipse) {
        painter->drawEllipse(rect);
    } else {
        qreal radius = m_shape == NodeShape::RoundedRect ? 12 : 0;
        painter->drawRoundedRect(rect, radius, radius);
    }

    // ===== 5. 绘制文字 =====
    QFont font;
    if (m_isCenterNode) {
        font.setPointSize(14);
        font.setBold(true);
    } else {
        font.setPointSize(11);
    }
    painter->setFont(font);
    painter->setPen(m_textColor);
    painter->drawText(rect, Qt::AlignCenter, m_text);
}

// ===== 悬停事件 =====
void MindNode::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_isHovered = true;
    update();   // 触发重绘
}

void MindNode::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    m_isHovered = false;
    update();
}

// ===== 项变化事件（用于监听位置变化等）=====
QVariant MindNode::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // 中心节点：不允许移动
    if (change == ItemPositionChange && m_isCenterNode) {
        return pos();
    }

    // 🆕 位置变化后：通知所有连线更新
    if (change == ItemPositionHasChanged) {
        for (MindEdge *edge : m_edges) {
            if (edge) edge->adjust();
        }
    }

    return QGraphicsItem::itemChange(change, value);
}

// ===== 属性设置 =====
void MindNode::setText(const QString &text)
{
    m_text = text;
    updateSize();
    update();
}

void MindNode::setBgColor(const QColor &color)
{
    m_bgColor = color;
    update();
}

void MindNode::setBorderColor(const QColor &color)
{
    m_borderColor = color;
    update();
}

void MindNode::setTextColor(const QColor &color)
{
    m_textColor = color;
    update();
}

void MindNode::setNodeShape(NodeShape shape)
{
    m_shape = shape;
    update();
}

void MindNode::setCenterNode(bool center)
{
    m_isCenterNode = center;

    if (center) {
        // 中心节点：橙色，不可拖动
        m_bgColor = QColor("#F39C12");
        m_borderColor = QColor("#B87200");
        setFlag(QGraphicsItem::ItemIsMovable, false);
        setCursor(Qt::ArrowCursor);
    } else {
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setCursor(Qt::PointingHandCursor);
    }

    updateSize();
    update();
}

// ===== 子节点管理 =====
void MindNode::addChild(MindNode *child)
{
    if (!child || m_childNodes.contains(child)) return;
    m_childNodes.append(child);
    child->m_parentNode = this;
}

void MindNode::removeChild(MindNode *child)
{
    m_childNodes.removeAll(child);
    if (child && child->m_parentNode == this) {
        child->m_parentNode = nullptr;
    }
}

// ===== 🆕 连线管理 =====
void MindNode::addEdge(MindEdge *edge)
{
    if (!edge || m_edges.contains(edge)) return;
    m_edges.append(edge);
}

void MindNode::removeEdge(MindEdge *edge)
{
    m_edges.removeAll(edge);
}


// ===== 🆕 右键菜单：添加/编辑/删除节点 =====
void MindNode::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    // ➕ 添加子节点
    QAction *addChildAct = menu.addAction(QObject::tr("➕ 添加子节点"));

    // ✏️ 编辑文字
    QAction *editAct = menu.addAction(QObject::tr("✏️ 编辑文字"));

    menu.addSeparator();

    // 🎨 修改颜色
    QAction *bgColorAct = menu.addAction(QObject::tr("🎨 背景颜色..."));
    QAction *textColorAct = menu.addAction(QObject::tr("🎨 文字颜色..."));

    // 🔄 形状子菜单
    QMenu *shapeMenu = menu.addMenu(QObject::tr("🔄 形状"));
    QAction *rectShapeAct = shapeMenu->addAction(QObject::tr("圆角矩形"));
    QAction *ellipseShapeAct = shapeMenu->addAction(QObject::tr("椭圆"));
    QAction *sharpRectShapeAct = shapeMenu->addAction(QObject::tr("矩形"));

    menu.addSeparator();

    // 🗑️ 删除（中心节点禁用）
    QAction *deleteAct = menu.addAction(QObject::tr("🗑️ 删除节点"));
    deleteAct->setEnabled(!m_isCenterNode);

    QAction *chosen = menu.exec(event->screenPos());

    if (!chosen) {
        event->accept();
        return;
    }

    // ===== 处理选择 =====
    if (chosen == addChildAct) {
        // 创建子节点
        MindNode *child = new MindNode(QObject::tr("新节点"));
        child->m_parentNode = this;
        m_childNodes.append(child);

        if (scene()) {
            scene()->addItem(child);

            // 位置：父节点右边
            int childCount = m_childNodes.size();
            qreal offsetX = 200;
            qreal offsetY = (childCount - 1) * 80 - (m_childNodes.size() - 1) * 40;
            child->setPos(pos().x() + offsetX, pos().y() + offsetY);

            // 创建连线
            MindEdge *edge = new MindEdge(this, child);
            addEdge(edge);
            child->addEdge(edge);
            scene()->addItem(edge);
        }
    }
    else if (chosen == editAct) {
        // 编辑文字（触发双击一样的效果）
        bool ok;
        QString newText = QInputDialog::getText(
            nullptr,
            QObject::tr("编辑节点"),
            QObject::tr("节点文字:"),
            QLineEdit::Normal,
            m_text,
            &ok);
        if (ok && !newText.trimmed().isEmpty()) {
            setText(newText.trimmed());
        }
    }
    else if (chosen == bgColorAct) {
        QColor color = QColorDialog::getColor(m_bgColor, nullptr,
                                              QObject::tr("选择背景颜色"));
        if (color.isValid()) {
            setBgColor(color);
        }
    }
    else if (chosen == textColorAct) {
        QColor color = QColorDialog::getColor(m_textColor, nullptr,
                                              QObject::tr("选择文字颜色"));
        if (color.isValid()) {
            setTextColor(color);
        }
    }
    else if (chosen == rectShapeAct) {
        setNodeShape(NodeShape::RoundedRect);
    }
    else if (chosen == ellipseShapeAct) {
        setNodeShape(NodeShape::Ellipse);
    }
    else if (chosen == sharpRectShapeAct) {
        setNodeShape(NodeShape::Rect);
    }
    else if (chosen == deleteAct) {
        // 🆕 删除节点（连同所有子孙）
        deleteWithChildren();
    }

    event->accept();
}

// ===== 🆕 双击编辑 =====
void MindNode::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    bool ok;
    QString newText = QInputDialog::getText(
        nullptr,
        QObject::tr("编辑节点"),
        QObject::tr("节点文字:"),
        QLineEdit::Normal,
        m_text,
        &ok);

    if (ok && !newText.trimmed().isEmpty()) {
        setText(newText.trimmed());
    }
}

// ===== 🆕 递归删除节点（连同所有子孙 + 连线）=====
void MindNode::deleteWithChildren()
{
    if (m_isCenterNode) return;   // 中心节点不能删

    // 第 1 步：递归删除所有子节点
    // 注意：不能用范围 for 循环，因为 removeChild 会修改 m_childNodes
    QVector<MindNode*> childrenCopy = m_childNodes;
    for (MindNode *child : childrenCopy) {
        if (child) {
            child->deleteWithChildren();
        }
    }

    // 第 2 步：删除所有连线
    QVector<MindEdge*> edgesCopy = m_edges;
    for (MindEdge *edge : edgesCopy) {
        if (!edge) continue;

        // 从两端节点的连线列表移除
        if (edge->sourceNode() && edge->sourceNode() != this) {
            edge->sourceNode()->removeEdge(edge);
        }
        if (edge->destNode() && edge->destNode() != this) {
            edge->destNode()->removeEdge(edge);
        }

        // 从场景移除
        if (scene()) {
            scene()->removeItem(edge);
        }
        delete edge;
    }
    m_edges.clear();

    // 第 3 步：从父节点的子列表移除
    if (m_parentNode) {
        m_parentNode->removeChild(this);
    }

    // 第 4 步：从场景移除自己
    if (scene()) {
        scene()->removeItem(this);
    }

    // 第 5 步：删除自己
    delete this;
}