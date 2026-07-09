#ifndef MINDMAPCOMMANDS_H
#define MINDMAPCOMMANDS_H

#include <QUndoCommand>
#include <QPointF>
#include <QString>

class MindNode;
class MindEdge;
class QGraphicsScene;

// ============================================================
// 移动节点命令
// ============================================================
class MoveNodeCommand : public QUndoCommand
{
public:
    MoveNodeCommand(MindNode *node,
                    const QPointF &oldPos,
                    const QPointF &newPos,
                    QUndoCommand *parent = nullptr);

    void undo() override;
    void redo() override;

    // ID 用于合并连续的移动（拖动时会有很多小移动）
    int id() const override { return 1001; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    MindNode *m_node;
    QPointF m_oldPos;
    QPointF m_newPos;
};

// ============================================================
// 添加节点命令
// ============================================================
class AddNodeCommand : public QUndoCommand
{
public:
    AddNodeCommand(QGraphicsScene *scene,
                   MindNode *parentNode,
                   const QString &text,
                   const QPointF &pos,
                   const QString &nodeId,
                   QUndoCommand *parent = nullptr);
    ~AddNodeCommand();

    void undo() override;
    void redo() override;

    MindNode *createdNode() const { return m_newNode; }

private:
    QGraphicsScene *m_scene;
    MindNode *m_parentNode;
    MindNode *m_newNode;         // 我们创建的节点
    MindEdge *m_newEdge;         // 我们创建的连线
    QString m_text;
    QPointF m_pos;
    QString m_nodeId;
    bool m_ownsNode;             // 撤销后，节点归本命令所有（用于析构清理）
};

// ============================================================
// 删除节点命令（含所有子孙）
// ============================================================
class DeleteNodeCommand : public QUndoCommand
{
public:
    DeleteNodeCommand(QGraphicsScene *scene,
                      MindNode *node,
                      QUndoCommand *parent = nullptr);
    ~DeleteNodeCommand();

    void undo() override;
    void redo() override;

private:
    // 保存节点的完整信息（用于重建）
    struct NodeSnapshot {
        QString id;
        QString text;
        QPointF pos;
        bool isCenter;
        int shape;
        QString bgColor;
        QString borderColor;
        QString textColor;
        QString parentId;
    };

    // 递归收集节点信息
    void collectNodeSnapshots(MindNode *node, QVector<NodeSnapshot> &snapshots);

    QGraphicsScene *m_scene;
    MindNode *m_rootNode;         // 要删除的根节点（可能有子孙）
    QVector<NodeSnapshot> m_snapshots;   // 所有被删除节点的快照
    bool m_deleted;               // 当前是否处于"已删除"状态
};

#endif // MINDMAPCOMMANDS_H