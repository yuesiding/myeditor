#include "mindmapcommands.h"
#include "mindnode.h"
#include "mindedge.h"

#include <QGraphicsScene>
#include <QHash>

// ============================================================
// MoveNodeCommand
// ============================================================
MoveNodeCommand::MoveNodeCommand(MindNode *node,
                                 const QPointF &oldPos,
                                 const QPointF &newPos,
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_node(node)
    , m_oldPos(oldPos)
    , m_newPos(newPos)
{
    setText(QObject::tr("移动节点"));
}

void MoveNodeCommand::undo()
{
    if (m_node) {
        m_node->setPos(m_oldPos);
    }
}

void MoveNodeCommand::redo()
{
    if (m_node) {
        m_node->setPos(m_newPos);
    }
}

bool MoveNodeCommand::mergeWith(const QUndoCommand *other)
{
    // 只合并同一个节点的连续移动
    const MoveNodeCommand *moveCmd = dynamic_cast<const MoveNodeCommand*>(other);
    if (!moveCmd || moveCmd->m_node != m_node) {
        return false;
    }

    // 合并：保留初始位置，更新为最终位置
    m_newPos = moveCmd->m_newPos;
    return true;
}

// ============================================================
// AddNodeCommand
// ============================================================
AddNodeCommand::AddNodeCommand(QGraphicsScene *scene,
                               MindNode *parentNode,
                               const QString &text,
                               const QPointF &pos,
                               const QString &nodeId,
                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
    , m_parentNode(parentNode)
    , m_newNode(nullptr)
    , m_newEdge(nullptr)
    , m_text(text)
    , m_pos(pos)
    , m_nodeId(nodeId)
    , m_ownsNode(false)
{
    setText(QObject::tr("添加节点"));
}

AddNodeCommand::~AddNodeCommand()
{
    // 如果撤销状态下析构（命令被从栈中丢弃），要清理节点
    if (m_ownsNode) {
        if (m_newEdge) delete m_newEdge;
        if (m_newNode) delete m_newNode;
    }
}

void AddNodeCommand::redo()
{
    if (!m_scene || !m_parentNode) return;

    if (!m_newNode) {
        // 第一次执行：创建节点
        m_newNode = new MindNode(m_text);
        m_newNode->setNodeId(m_nodeId);
        m_newNode->setPos(m_pos);
    }

    // 加到场景
    m_scene->addItem(m_newNode);

    // 加到父节点的子列表
    m_parentNode->addChild(m_newNode);

    // 创建连线（如果没有）
    if (!m_newEdge) {
        m_newEdge = new MindEdge(m_parentNode, m_newNode);
    }
    m_scene->addItem(m_newEdge);
    m_parentNode->addEdge(m_newEdge);
    m_newNode->addEdge(m_newEdge);

    m_ownsNode = false;   // 场景拥有节点
}

void AddNodeCommand::undo()
{
    if (!m_scene || !m_newNode) return;

    // 从连线列表移除
    if (m_newEdge) {
        m_parentNode->removeEdge(m_newEdge);
        m_newNode->removeEdge(m_newEdge);
        m_scene->removeItem(m_newEdge);
    }

    // 从父节点子列表移除
    m_parentNode->removeChild(m_newNode);

    // 从场景移除
    m_scene->removeItem(m_newNode);

    m_ownsNode = true;   // 我们拥有节点（内存在这里）
}

// ============================================================
// DeleteNodeCommand
// ============================================================
DeleteNodeCommand::DeleteNodeCommand(QGraphicsScene *scene,
                                     MindNode *node,
                                     QUndoCommand *parent)
    : QUndoCommand(parent)
    , m_scene(scene)
    , m_rootNode(node)
    , m_deleted(false)
{
    setText(QObject::tr("删除节点"));

    // 收集所有节点信息（用于以后恢复）
    if (node) {
        collectNodeSnapshots(node, m_snapshots);
    }
}

DeleteNodeCommand::~DeleteNodeCommand()
{
    // 如果处于"已删除"状态且命令被丢弃，就真的删除节点
    // 但为了简单，我们让节点在 redo() 中真正删除
}

void DeleteNodeCommand::collectNodeSnapshots(MindNode *node,
                                             QVector<NodeSnapshot> &snapshots)
{
    if (!node) return;

    NodeSnapshot snap;
    snap.id = node->nodeId();
    snap.text = node->text();
    snap.pos = node->pos();
    snap.isCenter = node->isCenterNode();
    snap.shape = static_cast<int>(node->nodeShape());
    snap.bgColor = node->bgColor().name();
    snap.borderColor = node->borderColor().name();
    snap.textColor = node->textColor().name();
    snap.parentId = node->parentNode() ? node->parentNode()->nodeId() : "";

    snapshots.append(snap);

    // 递归收集子节点
    for (MindNode *child : node->childNodes()) {
        collectNodeSnapshots(child, snapshots);
    }
}

void DeleteNodeCommand::redo()
{
    // 真正删除节点（含所有子孙）
    if (m_rootNode) {
        m_rootNode->deleteWithChildren();
        m_rootNode = nullptr;   // 已删除，指针无效
    }
    m_deleted = true;
}

void DeleteNodeCommand::undo()
{
    if (!m_scene) return;

    // 用快照重新创建所有节点
    QHash<QString, MindNode*> nodeMap;

    // 第 1 遍：创建所有节点
    for (const NodeSnapshot &snap : m_snapshots) {
        MindNode *node = new MindNode(snap.text);
        node->setNodeId(snap.id);
        node->setPos(snap.pos);
        node->setNodeShape(static_cast<NodeShape>(snap.shape));
        node->setBgColor(QColor(snap.bgColor));
        node->setBorderColor(QColor(snap.borderColor));
        node->setTextColor(QColor(snap.textColor));

        if (snap.isCenter) {
            node->setCenterNode(true);
        }

        m_scene->addItem(node);
        nodeMap[snap.id] = node;

        // 记住根节点
        if (snap.id == m_snapshots.first().id) {
            m_rootNode = node;
        }
    }

    // 第 2 遍：建立父子关系 + 创建连线
    // 恢复被删节点的根节点 → 场景中原来的父节点
    // 我们需要在场景里找 parentId 对应的节点
    for (const NodeSnapshot &snap : m_snapshots) {
        MindNode *node = nodeMap.value(snap.id);
        if (!node) continue;

        // 找父节点：先在快照里找，再在场景里找
        MindNode *parent = nodeMap.value(snap.parentId);
        if (!parent && !snap.parentId.isEmpty()) {
            // 父节点不在快照里，找场景里的
            for (QGraphicsItem *item : m_scene->items()) {
                MindNode *n = dynamic_cast<MindNode*>(item);
                if (n && n->nodeId() == snap.parentId) {
                    parent = n;
                    break;
                }
            }
        }

        if (parent) {
            parent->addChild(node);

            // 创建连线
            MindEdge *edge = new MindEdge(parent, node);
            parent->addEdge(edge);
            node->addEdge(edge);
            m_scene->addItem(edge);
        }
    }

    m_deleted = false;
}