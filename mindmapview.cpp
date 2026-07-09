#include "mindmapview.h"
#include "mindnode.h"
#include "mindedge.h"
#include <QGraphicsScene>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGraphicsItem>
#include <QInputDialog>
#include <QGraphicsSceneMouseEvent>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QInputDialog>
#include <QHash>
#include <QTimer>
#include <QUndoStack>
#include <QPixmap>
#include <QContextMenuEvent>
#include <QMenu>
#include "mindmapcommands.h"
#include "thememanager.h"

MindMapView::MindMapView(QWidget *parent)
    : QGraphicsView(parent)
    , m_currentScale(1.0)
    , m_modified(false)
    , m_nextNodeId(0)
    , m_undoStack(new QUndoStack(this))
{
    m_scene = new QGraphicsScene(this);
    m_scene->setSceneRect(-5000, -5000, 10000, 10000);
    setScene(m_scene);

    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setRenderHint(QPainter::TextAntialiasing);

    setDragMode(QGraphicsView::NoDrag);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    setBackgroundBrush(QColor("#F5F5F5"));

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setFocusPolicy(Qt::StrongFocus);
    centerOn(0, 0);

        // 🆕 监听主题变化
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &MindMapView::applyTheme);
    applyTheme();
    // 🆕 创建默认思维导图
    createDefaultMindMap();
        // 🆕 延迟连接场景变化信号（避免初始化时被触发）
    QTimer::singleShot(100, this, [this]() {
        connect(m_scene, &QGraphicsScene::changed,
                this, [this](const QList<QRectF> &) {
            setModified(true);
        });
    });
}
// ===== 滚轮缩放 =====
void MindMapView::wheelEvent(QWheelEvent *event)
{
    // 计算缩放因子
    const qreal scaleFactor = 1.15;

    if (event->angleDelta().y() > 0) {
        // 向上滚 → 放大
        scaleView(scaleFactor);
    } else {
        // 向下滚 → 缩小
        scaleView(1.0 / scaleFactor);
    }

    event->accept();
}

// ===== 应用缩放 =====
void MindMapView::scaleView(qreal factor)
{
    qreal newScale = m_currentScale * factor;

    // 限制缩放范围
    if (newScale < MIN_SCALE) {
        factor = MIN_SCALE / m_currentScale;
        newScale = MIN_SCALE;
    } else if (newScale > MAX_SCALE) {
        factor = MAX_SCALE / m_currentScale;
        newScale = MAX_SCALE;
    }

    scale(factor, factor);
    m_currentScale = newScale;
}

// ===== 鼠标按下（中键拖动）=====
void MindMapView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        // 中键 → 进入"手型拖动"模式
        setDragMode(QGraphicsView::ScrollHandDrag);

        // 转发一个"左键按下"事件（因为 ScrollHandDrag 只响应左键）
        QMouseEvent fake(QEvent::MouseButtonPress, event->pos(),
                         Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mousePressEvent(&fake);
        event->accept();
        return;
    }

    QGraphicsView::mousePressEvent(event);
}

// ===== 鼠标释放（结束拖动）=====
void MindMapView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        // 释放中键 → 结束拖动
        QMouseEvent fake(QEvent::MouseButtonRelease, event->pos(),
                         Qt::LeftButton, Qt::LeftButton, event->modifiers());
        QGraphicsView::mouseReleaseEvent(&fake);
        setDragMode(QGraphicsView::NoDrag);
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

// ===== 键盘事件 =====
void MindMapView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_0:
            resetView();
            event->accept();
            return;

        case Qt::Key_Plus:
        case Qt::Key_Equal:
            scaleView(1.2);
            event->accept();
            return;

        case Qt::Key_Minus:
            scaleView(1.0 / 1.2);
            event->accept();
            return;

        // Delete 键：删除选中的节点
        case Qt::Key_Delete: {
            QList<QGraphicsItem*> selected = m_scene->selectedItems();
            QVector<MindNode*> toDelete;
            for (QGraphicsItem *item : selected) {
                MindNode *node = dynamic_cast<MindNode*>(item);
                if (node && !node->isCenterNode()) {
                    toDelete.append(node);
                }
            }
            for (MindNode *node : toDelete) {
                DeleteNodeCommand *cmd = new DeleteNodeCommand(m_scene, node);
                m_undoStack->push(cmd);
            }
            event->accept();
            return;
        }

        // F2 键：编辑选中的节点
        case Qt::Key_F2: {
            QList<QGraphicsItem*> selected = m_scene->selectedItems();
            if (!selected.isEmpty()) {
                MindNode *node = dynamic_cast<MindNode*>(selected.first());
                if (node) {
                    node->startEdit();
                }
            }
            event->accept();
            return;
        }

        // 🆕 Tab 键：添加子节点
        case Qt::Key_Tab: {
            QList<QGraphicsItem*> selected = m_scene->selectedItems();
            MindNode *parentNode = nullptr;

            if (selected.isEmpty()) {
                // 没有选中：找中心节点
                for (QGraphicsItem *item : m_scene->items()) {
                    MindNode *n = dynamic_cast<MindNode*>(item);
                    if (n && n->isCenterNode()) {
                        parentNode = n;
                        break;
                    }
                }
            } else {
                parentNode = dynamic_cast<MindNode*>(selected.first());
            }

            if (parentNode) {
                MindNode *newNode = parentNode->addChildNode();
                if (newNode) {
                    // 选中新节点
                    m_scene->clearSelection();
                    newNode->setSelected(true);
                    // 立刻编辑
                    newNode->startEdit();
                }
            }
            event->accept();
            return;
        }

        // 🆕 Enter 键：添加同级节点
        case Qt::Key_Return:
        case Qt::Key_Enter: {
            QList<QGraphicsItem*> selected = m_scene->selectedItems();
            if (selected.isEmpty()) {
                event->accept();
                return;
            }

            MindNode *currentNode = dynamic_cast<MindNode*>(selected.first());
            if (!currentNode) {
                event->accept();
                return;
            }

            MindNode *newNode = nullptr;
            if (currentNode->isCenterNode()) {
                // 中心节点：加子节点
                newNode = currentNode->addChildNode();
            } else {
                // 普通节点：加兄弟
                newNode = currentNode->addSiblingNode();
            }

            if (newNode) {
                m_scene->clearSelection();
                newNode->setSelected(true);
                newNode->startEdit();
            }
            event->accept();
            return;
        }

        // 🆕 Ctrl+A：全选
        case Qt::Key_A: {
            if (event->modifiers() & Qt::ControlModifier) {
                for (QGraphicsItem *item : m_scene->items()) {
                    if (dynamic_cast<MindNode*>(item)) {
                        item->setSelected(true);
                    }
                }
                event->accept();
                return;
            }
            break;
        }
    }

    QGraphicsView::keyPressEvent(event);
}

// ===== 重置视图 =====
void MindMapView::resetView()
{
    resetTransform();
    m_currentScale = 1.0;
    centerOn(0, 0);
}

// ===== 缩放到适合窗口 =====
void MindMapView::zoomToFit()
{
    if (m_scene->items().isEmpty()) {
        resetView();
        return;
    }

    QRectF bounds = m_scene->itemsBoundingRect();
    if (bounds.isEmpty()) {
        resetView();
        return;
    }

    // 留一些边距
    bounds.adjust(-50, -50, 50, 50);
    fitInView(bounds, Qt::KeepAspectRatio);

    // 计算实际缩放比
    m_currentScale = transform().m11();
}

// ============================================================
// 🆕 创建默认思维导图（一个中心节点 + 几个子节点）
// ============================================================
void MindMapView::createDefaultMindMap()
{
    m_scene->clear();
    m_nextNodeId = 0;

    // 中心节点
    MindNode *centerNode = new MindNode(tr("中心主题"));
    centerNode->setCenterNode(true);
    centerNode->setPos(0, 0);
    centerNode->setNodeId(generateNodeId());
    m_scene->addItem(centerNode);

    // 3 个子节点
    QVector<QPointF> positions = {
        QPointF(-250, -100),
        QPointF(-250, 100),
        QPointF(250, 0)
    };

    QStringList names = { tr("子节点 1"), tr("子节点 2"), tr("子节点 3") };

    for (int i = 0; i < 3; ++i) {
        MindNode *child = new MindNode(names[i]);
        child->setPos(positions[i]);
        child->setNodeId(generateNodeId());
        m_scene->addItem(child);

        // 建立父子关系
        centerNode->addChild(child);

        // 创建连线
        MindEdge *edge = new MindEdge(centerNode, child);
        centerNode->addEdge(edge);
        child->addEdge(edge);
        m_scene->addItem(edge);
    }

    m_modified = false;
    m_currentFile.clear();
    if (m_undoStack) m_undoStack->clear();
    emit fileInfoChanged();
}

// ===== 生成新 ID =====
QString MindMapView::generateNodeId()
{
    return QString("node_%1").arg(m_nextNodeId++);
}

// ===== 新建 =====
void MindMapView::newMindMap()
{
    if (!maybeSave()) return;
    createDefaultMindMap();
    resetView();
}

// ===== 设置当前文件 =====
void MindMapView::setCurrentFile(const QString &path)
{
    m_currentFile = path;
    emit fileInfoChanged();
}

// ===== 设置修改状态 =====
void MindMapView::setModified(bool m)
{
    if (m_modified != m) {
        m_modified = m;
        emit fileInfoChanged();
    }
}

// ===== 关闭前询问 =====
bool MindMapView::maybeSave()
{
    if (!m_modified) return true;

    auto ret = QMessageBox::warning(
        this,
        tr("提示"),
        tr("当前思维导图已修改，是否保存？"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (ret == QMessageBox::Save) {
        // 需要外部触发保存
        return false;   // 让 MainWindow 决定
    }
    if (ret == QMessageBox::Cancel) return false;
    return true;   // Discard
}

// ===== 保存到文件 =====
bool MindMapView::saveToFile(const QString &filePath)
{
    QJsonObject root;
    root["version"] = "1.0";
    root["type"] = "MyEditor MindMap";

    // 收集所有节点
    QJsonArray nodesArray;
    for (QGraphicsItem *item : m_scene->items()) {
        MindNode *node = dynamic_cast<MindNode*>(item);
        if (!node) continue;

        // 确保有 ID
        if (node->nodeId().isEmpty()) {
            node->setNodeId(generateNodeId());
        }

        QJsonObject nodeObj;
        nodeObj["id"] = node->nodeId();
        nodeObj["text"] = node->text();
        nodeObj["x"] = node->pos().x();
        nodeObj["y"] = node->pos().y();
        nodeObj["isCenter"] = node->isCenterNode();
        nodeObj["shape"] = static_cast<int>(node->nodeShape());
        nodeObj["bgColor"] = node->bgColor().name();
        nodeObj["borderColor"] = node->borderColor().name();
        nodeObj["textColor"] = node->textColor().name();

        // 父节点 ID
        if (node->parentNode()) {
            if (node->parentNode()->nodeId().isEmpty()) {
                node->parentNode()->setNodeId(generateNodeId());
            }
            nodeObj["parentId"] = node->parentNode()->nodeId();
        } else {
            nodeObj["parentId"] = "";
        }

        nodesArray.append(nodeObj);
    }

    root["nodes"] = nodesArray;

    QJsonDocument doc(root);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("保存失败"),
                             tr("无法写入文件：%1").arg(file.errorString()));
        return false;
    }

    file.write(data);
    file.close();

    setCurrentFile(filePath);
    setModified(false);
    return true;
}

// ===== 从文件加载 =====
bool MindMapView::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("无法打开文件：%1").arg(file.errorString()));
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("JSON 解析错误：%1").arg(parseError.errorString()));
        return false;
    }

    QJsonObject root = doc.object();
    if (root["type"].toString() != "MyEditor MindMap") {
        QMessageBox::warning(this, tr("打开失败"),
                             tr("不是有效的思维导图文件"));
        return false;
    }

    // 清空当前场景
    m_scene->clear();
    m_nextNodeId = 0;

    // 第 1 遍：创建所有节点
    QHash<QString, MindNode*> nodeMap;
    QJsonArray nodesArray = root["nodes"].toArray();

    for (const QJsonValue &v : nodesArray) {
        QJsonObject obj = v.toObject();

        MindNode *node = new MindNode(obj["text"].toString());
        node->setNodeId(obj["id"].toString());
        node->setPos(obj["x"].toDouble(), obj["y"].toDouble());
        node->setNodeShape(static_cast<NodeShape>(obj["shape"].toInt()));
        node->setBgColor(QColor(obj["bgColor"].toString()));
        node->setBorderColor(QColor(obj["borderColor"].toString()));
        node->setTextColor(QColor(obj["textColor"].toString()));

        if (obj["isCenter"].toBool()) {
            node->setCenterNode(true);
        }

        m_scene->addItem(node);
        nodeMap[node->nodeId()] = node;
    }

    // 第 2 遍：建立父子关系 + 创建连线
    for (const QJsonValue &v : nodesArray) {
        QJsonObject obj = v.toObject();
        QString id = obj["id"].toString();
        QString parentId = obj["parentId"].toString();

        if (parentId.isEmpty()) continue;

        MindNode *node = nodeMap.value(id);
        MindNode *parent = nodeMap.value(parentId);

        if (node && parent) {
            parent->addChild(node);

            // 创建连线
            MindEdge *edge = new MindEdge(parent, node);
            parent->addEdge(edge);
            node->addEdge(edge);
            m_scene->addItem(edge);
        }
    }

    // 更新 ID 计数器（避免新节点 ID 冲突）
    for (const QString &id : nodeMap.keys()) {
        if (id.startsWith("node_")) {
            bool ok;
            int num = id.mid(5).toInt(&ok);
            if (ok && num >= m_nextNodeId) {
                m_nextNodeId = num + 1;
            }
        }
    }

    setCurrentFile(filePath);
    setModified(false);
    zoomToFit();
        if (m_undoStack) m_undoStack->clear();
    return true;
}

// ===== 🆕 双击空白处创建自由节点 =====
void MindMapView::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 检查点击位置是否有节点
    QGraphicsItem *item = itemAt(event->pos());
    if (item) {
        // 点在节点上，让节点处理
        QGraphicsView::mouseDoubleClickEvent(event);
        return;
    }

    // 空白处双击：暂时不做（避免创建游离节点）
    // 提示用户用右键
    // 可以在这里加提示，但暂时留空
    QGraphicsView::mouseDoubleClickEvent(event);
}

// ============================================================
// 🆕 撤销/重做
// ============================================================
void MindMapView::undo()
{
    if (m_undoStack->canUndo()) {
        m_undoStack->undo();
    }
}

void MindMapView::redo()
{
    if (m_undoStack->canRedo()) {
        m_undoStack->redo();
    }
}

// ============================================================
// 🆕 导出为图片
// ============================================================
bool MindMapView::exportToImage(const QString &filePath)
{
    if (!m_scene) return false;

    // 计算所有节点的包围盒
    QRectF sceneRect = m_scene->itemsBoundingRect();
    if (sceneRect.isEmpty()) {
        return false;
    }

    // 加一些边距
    sceneRect.adjust(-30, -30, 30, 30);

    // 创建高清图片（2x 分辨率）
    qreal ratio = 2.0;
    int w = static_cast<int>(sceneRect.width() * ratio);
    int h = static_cast<int>(sceneRect.height() * ratio);

    QPixmap pixmap(w, h);
    pixmap.fill(Qt::white);   // 白色背景

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.setRenderHint(QPainter::TextAntialiasing);

    // 缩放画布匹配图片
    painter.scale(ratio, ratio);

    // 让场景绘制到我们的 painter
    m_scene->render(&painter, QRectF(0, 0, sceneRect.width(), sceneRect.height()),
                    sceneRect);
    painter.end();

    // 保存
    return pixmap.save(filePath, "PNG");
}
// ============================================================
// 🆕 应用主题
// ============================================================
void MindMapView::applyTheme()
{
    const Theme &theme = ThemeManager::instance().currentTheme();
    bool isDark = theme.editorBackground.lightness() < 128;

    // 画布背景色
    QColor bgColor;
    if (isDark) {
        bgColor = QColor("#1E1E1E");   // 深色背景
    } else {
        bgColor = QColor("#F5F5F5");   // 浅灰背景
    }
    setBackgroundBrush(bgColor);

    // 触发所有节点重绘（它们会读主题色）
    if (m_scene) {
        for (QGraphicsItem *item : m_scene->items()) {
            item->update();
        }
    }
}

// ============================================================
// 🆕 右键菜单（空白处）
// ============================================================
void MindMapView::contextMenuEvent(QContextMenuEvent *event)
{
    // 如果右键在节点上，让节点处理
    QGraphicsItem *item = itemAt(event->pos());
    if (item) {
        QGraphicsView::contextMenuEvent(event);
        return;
    }

    // 右键空白处：显示画布菜单
    QMenu menu(this);

    QAction *newAct = menu.addAction(tr("🧠 新建思维导图"));
    QAction *fitAct = menu.addAction(tr("🎯 适应窗口"));
    QAction *resetAct = menu.addAction(tr("↩ 重置视图"));
    QAction *centerAct = menu.addAction(tr("🎯 居中中心节点"));
    menu.addSeparator();

    QAction *undoAct = menu.addAction(tr("↶ 撤销"));
    undoAct->setEnabled(m_undoStack && m_undoStack->canUndo());

    QAction *redoAct = menu.addAction(tr("↷ 重做"));
    redoAct->setEnabled(m_undoStack && m_undoStack->canRedo());

    QAction *chosen = menu.exec(event->globalPos());

        if (chosen == newAct) {
        newMindMap();
    } else if (chosen == fitAct) {
        zoomToFit();
    } else if (chosen == resetAct) {
        resetView();
    } else if (chosen == centerAct) {    
        centerCenterNode();
    } else if (chosen == undoAct) {
        undo();
    } else if (chosen == redoAct) {
        redo();
    }
}

// ===== 🆕 阻止 Tab 键切换焦点 =====
bool MindMapView::focusNextPrevChild(bool next)
{
    Q_UNUSED(next);
    return false;   // 让 Tab 键交给 keyPressEvent 处理
}

// ===== 🆕 触发 AI 展开节点（供 MindNode 调用）=====
void MindMapView::requestAiExpandForNode(MindNode *node)
{
    if (!node) return;

    // 通过 window() 找 MainWindow，调用它的方法
    QWidget *w = window();
    if(w){
        // 用属性传递指针
        QMetaObject::invokeMethod(w, "requestAiExpandFromNode",
            Q_ARG(QVariant, QVariant::fromValue(reinterpret_cast<quintptr>(node))));
    }
}

// ===== 🆕 居中中心节点 =====
void MindMapView::centerCenterNode()
{
    if (!m_scene) return;

    // 找到中心节点
    MindNode *centerNode = nullptr;
    for (QGraphicsItem *item : m_scene->items()) {
        MindNode *n = dynamic_cast<MindNode*>(item);
        if (n && n->isCenterNode()) {
            centerNode = n;
            break;
        }
    }

    if (!centerNode) return;

    // 移到 (0, 0)
    centerNode->setPos(0, 0);

    // 视图也居中
    centerOn(0, 0);
}