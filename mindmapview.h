#ifndef MINDMAPVIEW_H
#define MINDMAPVIEW_H

#include <QGraphicsView>

class QGraphicsScene;
class QWheelEvent;
class QMouseEvent;
class QKeyEvent;
class MindNode;
class QUndoStack;

class MindMapView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MindMapView(QWidget *parent = nullptr);

    // 视图控制
    void resetView();
    void zoomToFit();

    // 🆕 文件操作
    void newMindMap();                              // 新建
    bool loadFromFile(const QString &filePath);     // 从文件加载
    bool saveToFile(const QString &filePath);       // 保存到文件

    // 🆕 当前文件路径
    QString currentFile() const { return m_currentFile; }
    void setCurrentFile(const QString &path);

    // 🆕 是否有未保存的修改
    bool isModified() const { return m_modified; }
    void setModified(bool m);

    // 🆕 关闭前询问保存（返回 true 表示可以继续）
    bool maybeSave();
        // 🆕 撤销/重做
    QUndoStack *undoStack() const { return m_undoStack; }
    void undo();
    void redo();
        // 🆕 导出为图片
    bool exportToImage(const QString &filePath);
        // 🆕 供节点调用
    void requestAiExpandForNode(MindNode *node);
        // 🆕 居中中心节点（拉回 (0, 0)）
    void centerCenterNode();
signals:
    // 🆕 文件信息变化（通知 MainWindow 更新标题）
    void fileInfoChanged();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    // 🆕 双击画布空白 → 创建自由节点
    void mouseDoubleClickEvent(QMouseEvent *event) override;
        // 🆕 右键菜单（空白处）
    void contextMenuEvent(QContextMenuEvent *event) override;
        // 🆕 让 Tab 键传给 keyPressEvent（否则 Tab 会被吃掉切换焦点）
    bool focusNextPrevChild(bool next) override;
private:
    void scaleView(qreal factor);

    // 🆕 创建默认的中心节点（新建时用）
    void createDefaultMindMap();

    // 🆕 生成新的节点 ID
    QString generateNodeId();

    QGraphicsScene *m_scene;
    qreal m_currentScale;

    // 🆕
    QString m_currentFile;
    bool m_modified;
    int m_nextNodeId;   // ID 计数器

    static constexpr qreal MIN_SCALE = 0.1;
    static constexpr qreal MAX_SCALE = 5.0;
        // 🆕 撤销栈
    QUndoStack *m_undoStack;

private slots:
    void applyTheme();// 🆕 应用主题
};

#endif // MINDMAPVIEW_H