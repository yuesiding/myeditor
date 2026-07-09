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
#include "mindmapview.h"
#include "mindmapcommands.h"
#include <QDateTime>
#include <QGraphicsView>
#include <QMetaObject>
#include <QGuiApplication>

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
    , m_isDragging(false)
    , m_collapsed(false)
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
        // 🆕 设置 tooltip
    setToolTip(QObject::tr("双击编辑 | 右键更多操作"));
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
    qreal shadow = 4;
    qreal iconExtra = m_childNodes.isEmpty() ? 0 : 10;   // 🆕 图标预留空间
    return QRectF(-m_size.width() / 2 - shadow,
                  -m_size.height() / 2 - shadow,
                  m_size.width() + shadow * 2 + iconExtra,
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
        // ===== 🆕 6. 绘制折叠图标（只在有子节点时）=====
    if (!m_childNodes.isEmpty()) {
        // 图标位置：节点右侧
        qreal iconSize = 14;
        qreal iconX = m_size.width() / 2 - 4;
        qreal iconY = -iconSize / 2;
        QRectF iconRect(iconX, iconY, iconSize, iconSize);

        // 图标背景（白色圆）
        painter->setPen(QPen(m_borderColor, 1));
        painter->setBrush(QColor("#FFFFFF"));
        painter->drawEllipse(iconRect);

        // 图标符号
        painter->setPen(QPen(m_borderColor, 2));
        QFont iconFont;
        iconFont.setPixelSize(10);
        iconFont.setBold(true);
        painter->setFont(iconFont);

        QString icon = m_collapsed ? "+" : "-";
        painter->drawText(iconRect, Qt::AlignCenter, icon);
    }
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
    // 🆕 中心节点：只有按住 Shift 才能移动
    if (change == ItemPositionChange && m_isCenterNode) {
        // 检查是否按住 Shift
        if (!(QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)) {
            // 没按 Shift，禁止移动
            return pos();
        }
        // 按了 Shift，允许移动
    }

    // 位置变化后：通知所有连线更新
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
        m_bgColor = QColor("#F39C12");
        m_borderColor = QColor("#B87200");
        // 🆕 允许移动（但 itemChange 会检查 Shift 键）
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setCursor(Qt::PointingHandCursor);
    } else {
        setFlag(QGraphicsItem::ItemIsMovable, true);
        setCursor(Qt::PointingHandCursor);
    }

    updateSize();
    update();
    if (center) {
        setToolTip(QObject::tr("中心节点\n按住 Shift 拖动可移动位置"));
    } else {
        setToolTip(QObject::tr("双击编辑 | 右键更多操作"));
    }
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
        // 🆕 中心节点特有：居中
    QAction *centerAct = nullptr;
    if (m_isCenterNode) {
        centerAct = menu.addAction(QObject::tr("🎯 回到中心位置 (0, 0)"));
        menu.addSeparator();
    }

    // ➕ 添加子节点
    QAction *addChildAct = menu.addAction(QObject::tr("➕ 添加子节点"));

    // ✏️ 编辑文字
    QAction *editAct = menu.addAction(QObject::tr("✏️ 编辑文字"));
        // 🆕 AI 展开
    QAction *aiExpandAct = menu.addAction(QObject::tr("🤖 AI 展开这个话题"));
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

        if (centerAct && chosen == centerAct) {
        // 通过 MindMapView 居中
        if (scene()) {
            QList<QGraphicsView*> views = scene()->views();
            for (auto *v : views) {
                MindMapView *mv = qobject_cast<MindMapView*>(v);
                if (mv) {
                    mv->centerCenterNode();
                    break;
                }
            }
        }
        event->accept();
        return;
    }

    // ===== 处理选择 =====
        if (chosen == addChildAct) {
        if (!scene()) { event->accept(); return; }

        // 计算子节点位置
        int childCount = m_childNodes.size() + 1;
        qreal offsetX = 200;
        qreal offsetY = (childCount - 1) * 80 - (childCount / 2) * 40;
        QPointF newPos(pos().x() + offsetX, pos().y() + offsetY);

        // 生成新 ID（用时间戳保证唯一）
        QString newId = QString("node_%1").arg(QDateTime::currentMSecsSinceEpoch());

        // 用命令模式添加
        QList<QGraphicsView*> views = scene()->views();
        MindMapView *view = nullptr;
        for (auto *v : views) {
            view = qobject_cast<MindMapView*>(v);
            if (view) break;
        }

        if (view) {
            AddNodeCommand *cmd = new AddNodeCommand(
                scene(), this, QObject::tr("新节点"), newPos, newId);
            view->undoStack()->push(cmd);
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
        // 用命令模式删除
        QList<QGraphicsView*> views = scene() ? scene()->views() : QList<QGraphicsView*>();
        MindMapView *view = nullptr;
        for (auto *v : views) {
            view = qobject_cast<MindMapView*>(v);
            if (view) break;
        }

        if (view) {
            DeleteNodeCommand *cmd = new DeleteNodeCommand(scene(), this);
            view->undoStack()->push(cmd);
        } else {
            // 备用方案：直接删除
            deleteWithChildren();
        }
    }        else if (chosen == aiExpandAct) {
        if (scene()) {
            QList<QGraphicsView*> views = scene()->views();
            for (auto *v : views) {
                MindMapView *mv = qobject_cast<MindMapView*>(v);
                if (mv) {
                    mv->requestAiExpandForNode(this);
                    break;
                }
            }
        }
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

// ===== 🆕 记录拖动起始位置 =====
void MindNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // 先检测是否点击了折叠图标
    if (event->button() == Qt::LeftButton && !m_childNodes.isEmpty()) {
        if (isPointOnFoldIcon(event->pos())) {
            toggleCollapsed();
            event->accept();
            return;
        }
    }

    // 🆕 记录拖动起始位置
    // 中心节点：只有按住 Shift 才记录（可拖动）
    if (event->button() == Qt::LeftButton) {
        if (!m_isCenterNode) {
            m_dragStartPos = pos();
            m_isDragging = true;
        } else if (event->modifiers() & Qt::ShiftModifier) {
            m_dragStartPos = pos();
            m_isDragging = true;
        }
    }
    QGraphicsItem::mousePressEvent(event);
}
// ===== 🆕 拖动结束，生成移动命令 =====
void MindNode::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isDragging && !m_isCenterNode) {
        m_isDragging = false;
        QPointF newPos = pos();

        // 如果位置有变化，生成命令
        if (newPos != m_dragStartPos && scene()) {
            QList<QGraphicsView*> views = scene()->views();
            for (auto *v : views) {
                MindMapView *mv = qobject_cast<MindMapView*>(v);
                if (mv) {
                    // 先把节点位置改回起点，再通过命令移回来（保证撤销时能回去）
                    // 但已经在 newPos 了，命令的 redo 是设为 newPos
                    // undo 是设回 m_dragStartPos
                    // 所以直接推命令
                    MoveNodeCommand *cmd = new MoveNodeCommand(
                        this, m_dragStartPos, newPos);
                    // 不调用 redo（因为节点已经在 newPos）
                    // 用 setSkipRedo 的技巧不好，我们让 redo 幂等
                    mv->undoStack()->push(cmd);
                    break;
                }
            }
        }
    }
    QGraphicsItem::mouseReleaseEvent(event);
}

// ============================================================
// 🆕 折叠/展开
// ============================================================
bool MindNode::isPointOnFoldIcon(const QPointF &localPos) const
{
    if (m_childNodes.isEmpty()) return false;

    qreal iconSize = 14;
    qreal iconX = m_size.width() / 2 - 4;
    qreal iconY = -iconSize / 2;
    QRectF iconRect(iconX, iconY, iconSize, iconSize);

    return iconRect.contains(localPos);
}

void MindNode::setCollapsed(bool collapsed)
{
    if (m_collapsed == collapsed) return;
    m_collapsed = collapsed;
    setChildrenVisible(!collapsed);
    update();
}

void MindNode::toggleCollapsed()
{
    setCollapsed(!m_collapsed);
}

// 递归显示/隐藏所有子孙节点 + 连线
void MindNode::setChildrenVisible(bool visible)
{
    for (MindNode *child : m_childNodes) {
        if (!child) continue;

        child->setVisible(visible);

        // 隐藏连接父子的连线
        for (MindEdge *edge : m_edges) {
            if (edge && edge->destNode() == child) {
                edge->setVisible(visible);
            }
        }

        // 递归处理子孙
        if (visible && !child->m_collapsed) {
            // 展开时：如果子节点自己没被折叠，展开它的子孙
            child->setChildrenVisible(true);
        } else if (!visible) {
            // 折叠时：无条件隐藏所有子孙
            child->setChildrenVisible(false);
        }
    }
}

// ============================================================
// 🆕 快速添加节点（供快捷键使用）
// ============================================================

// 添加子节点
MindNode *MindNode::addChildNode(const QString &text)
{
    if (!scene()) return nullptr;

    // 计算新节点位置
    int childCount = m_childNodes.size() + 1;
    qreal offsetX = 200;
    qreal offsetY = (childCount - 1) * 80 - (childCount / 2) * 40;
    QPointF newPos(pos().x() + offsetX, pos().y() + offsetY);

    // 生成 ID
    QString newId = QString("node_%1").arg(QDateTime::currentMSecsSinceEpoch());

    // 通过 MindMapView 的 UndoStack 添加
    QList<QGraphicsView*> views = scene()->views();
    MindMapView *view = nullptr;
    for (auto *v : views) {
        view = qobject_cast<MindMapView*>(v);
        if (view) break;
    }

    QString nodeText = text.isEmpty() ? QObject::tr("新节点") : text;

    if (view) {
        AddNodeCommand *cmd = new AddNodeCommand(
            scene(), this, nodeText, newPos, newId);
        view->undoStack()->push(cmd);
        return cmd->createdNode();
    }

    return nullptr;
}

// 添加同级节点（兄弟）
MindNode *MindNode::addSiblingNode(const QString &text)
{
    // 中心节点没有兄弟
    if (m_isCenterNode || !m_parentNode) return nullptr;

    return m_parentNode->addChildNode(text);
}

// 触发编辑
void MindNode::startEdit()
{
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