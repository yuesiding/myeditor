#include "tasklistwidget.h"
#include "thememanager.h"
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QComboBox>
#include <QApplication>
#include <QClipboard>

TaskListWidget::TaskListWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();

    // 🆕 尝试从默认位置加载
    loadFromDefaultLocation();

    // 如果加载后没有任何任务，添加示例任务（首次使用）
    if (m_model->rowCount() == 0) {
        addTaskItem(Task("欢迎使用任务清单！点击复选框标记完成", TaskPriority::Normal));
        addTaskItem(Task("试试右键任务查看更多操作", TaskPriority::Low));
    }
        // 🆕 连接主题变化
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &TaskListWidget::applyTheme);

    // 立即应用当前主题
    applyTheme();
    updateStats();
}

void TaskListWidget::setupUi()
{
    // ===== 标题 =====
    m_titleLabel = new QLabel(tr("📋 任务清单"), this);
    m_titleLabel->setStyleSheet(
        "QLabel { padding: 6px; font-weight: bold; font-size: 13px; }"
    );

    // ===== 输入区 =====
    m_inputField = new QLineEdit(this);
    m_inputField->setPlaceholderText(tr("输入新任务，回车添加..."));

    m_addButton = new QPushButton(tr("➕ 添加"), this);
    connect(m_addButton, &QPushButton::clicked,
            this, &TaskListWidget::onAddTaskClicked);
    connect(m_inputField, &QLineEdit::returnPressed,
            this, &TaskListWidget::onAddTaskClicked);

        // 🆕 筛选下拉框
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItem(tr("📋 全部"), ShowAll);
    m_filterCombo->addItem(tr("⏳ 未完成"), ShowPending);
    m_filterCombo->addItem(tr("✅ 已完成"), ShowCompleted);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TaskListWidget::onFilterChanged);
    
    // ===== 任务列表 =====
    m_model = new QStandardItemModel(this);
    m_listView = new QListView(this);
    m_listView->setModel(m_model);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
        // 样式（简单适配主题）
    m_listView->setStyleSheet(
        "QListView { "
        "  border: 1px solid palette(mid); "
        "  background: palette(base); "
        "  padding: 4px; "
        "} "
        "QListView::item { "
        "  padding: 6px; "
        "  border-bottom: 1px solid palette(midlight); "
        "} "
        "QListView::item:hover { background: palette(highlight); color: palette(highlighted-text); } "
        "QListView::item:selected { background: palette(highlight); color: palette(highlighted-text); }"
    );

    // 🆕 复选框变化时更新样式
    connect(m_model, &QStandardItemModel::itemChanged,
            this, &TaskListWidget::onItemChanged);

    // 🆕 右键菜单
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_listView, &QListView::customContextMenuRequested,
            this, &TaskListWidget::onCustomContextMenu);

    // 🆕 双击编辑
    connect(m_listView, &QListView::doubleClicked,
            this, &TaskListWidget::onItemDoubleClicked);

    // ===== 底部按钮 =====
    m_removeCompletedButton = new QPushButton(tr("🗑️ 删除已完成"), this);
    connect(m_removeCompletedButton, &QPushButton::clicked,
            this, &TaskListWidget::onRemoveCompletedClicked);

    // ===== 统计标签 =====
    m_statsLabel = new QLabel(this);
    m_statsLabel->setStyleSheet(
        "QLabel { padding: 4px; color: #666; font-size: 11px; }"
    );

    // ===== 布局 =====
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(m_inputField, 1);
    inputLayout->addWidget(m_addButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->addWidget(m_titleLabel);
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(m_filterCombo);
    mainLayout->addWidget(m_listView, 1);
    mainLayout->addWidget(m_removeCompletedButton);
    mainLayout->addWidget(m_statsLabel);

    setLayout(mainLayout);
}

// ===== 添加任务到列表 =====
void TaskListWidget::addTaskItem(const Task &task)
{
    QStandardItem *item = new QStandardItem();
    item->setText(task.title);
    item->setCheckable(true);
    item->setCheckState(task.completed ? Qt::Checked : Qt::Unchecked);

    // 存储优先级到 UserRole
    item->setData(static_cast<int>(task.priority), PriorityRole);

    m_model->appendRow(item);
    updateItemStyle(item);
}

void TaskListWidget::updateItemStyle(QStandardItem *item)
{
    if (!item) return;

    bool completed = (item->checkState() == Qt::Checked);
    int priorityInt = item->data(PriorityRole).toInt();
    TaskPriority priority = static_cast<TaskPriority>(priorityInt);

    // 字体：完成任务加删除线
    QFont f = item->font();
    f.setStrikeOut(completed);
    item->setFont(f);

    // 🆕 判断是否暗主题
    const Theme &theme = ThemeManager::instance().currentTheme();
    bool isDark = theme.editorBackground.lightness() < 128;

    // 颜色：完成任务变灰，未完成按优先级
    if (completed) {
        // 暗主题下浅一点，亮主题下深一点
        item->setForeground(QBrush(isDark ? QColor("#666") : QColor("#999")));
    } else {
        switch (priority) {
            case TaskPriority::High:
                item->setForeground(QBrush(isDark ? QColor("#FF6B6B") : QColor("#C0392B")));
                break;
            case TaskPriority::Normal:
                item->setForeground(QBrush(isDark ? QColor("#E0E0E0") : QColor("#2C3E50")));
                break;
            case TaskPriority::Low:
                item->setForeground(QBrush(isDark ? QColor("#888") : QColor("#7F8C8D")));
                break;
        }
    }

    // 优先级图标（在文字前加）
    QString prefix;
    switch (priority) {
        case TaskPriority::High:   prefix = "🔴 "; break;
        case TaskPriority::Normal: prefix = "🔵 "; break;
        case TaskPriority::Low:    prefix = "⚪ "; break;
    }

    QString text = item->text();
    if (text.startsWith("🔴 ") || text.startsWith("🔵 ") || text.startsWith("⚪ ")) {
        text = text.mid(3);
    }
    item->setText(prefix + text);
}

void TaskListWidget::onAddTaskClicked()
{
    QString title = m_inputField->text().trimmed();
    if (title.isEmpty()) return;

    addTaskItem(Task(title, TaskPriority::Normal));
    m_inputField->clear();
    updateStats();
    saveToDefaultLocation();   // 🆕 自动保存
}

// ===== 🆕 删除已完成按钮 =====
void TaskListWidget::onRemoveCompletedClicked()
{
    // 统计要删除的
    int completedNum = completedCount();
    if (completedNum == 0) {
        QMessageBox::information(this, tr("提示"),
                                  tr("没有已完成的任务"));
        return;
    }

    // 确认对话框
    auto ret = QMessageBox::question(
        this,
        tr("确认删除"),
        tr("确定要删除 %1 个已完成的任务吗？").arg(completedNum),
        QMessageBox::Yes | QMessageBox::No
    );

    if (ret != QMessageBox::Yes) return;

    // 从后往前删除（避免索引错乱）
    for (int i = m_model->rowCount() - 1; i >= 0; --i) {
        QStandardItem *item = m_model->item(i);
        if (item && item->checkState() == Qt::Checked) {
            m_model->removeRow(i);
        }
    }

    updateStats();
     saveToDefaultLocation();   // 🆕 自动保存
}

// ===== 🆕 复选框变化时 =====
void TaskListWidget::onItemChanged(QStandardItem *item)
{
    // 更新样式（加/去删除线）
    updateItemStyle(item);
    updateStats();
    saveToDefaultLocation(); 
    applyFilter();
}

// ===== 🆕 右键菜单 =====
void TaskListWidget::onCustomContextMenu(const QPoint &pos)
{
    QModelIndex index = m_listView->indexAt(pos);
    if (!index.isValid()) return;

    QStandardItem *item = m_model->itemFromIndex(index);
    if (!item) return;

    QMenu menu(this);

    // 编辑
    QAction *editAction = menu.addAction(tr("✏️ 编辑"));
    connect(editAction, &QAction::triggered, [this, index]() {
        onItemDoubleClicked(index);
    });

        // 🆕 复制任务文本
    QAction *copyAction = menu.addAction(tr("📋 复制"));
    connect(copyAction, &QAction::triggered, [this, item]() {
        // 去掉优先级前缀再复制
        QString text = item->text();
        if (text.startsWith("🔴 ") ||
            text.startsWith("🔵 ") ||
            text.startsWith("⚪ ")) {
            text = text.mid(3);
        }
        QApplication::clipboard()->setText(text);
    });
    // 优先级子菜单
    menu.addSeparator();
    QMenu *priorityMenu = menu.addMenu(tr("优先级"));

    // 🔴 高优先级
    QAction *highAction = priorityMenu->addAction(tr("🔴 高"));
    connect(highAction, &QAction::triggered, [this, item]() {
        item->setData(static_cast<int>(TaskPriority::High), PriorityRole);
        updateItemStyle(item);
        saveToDefaultLocation();   // 🆕 自动保存
    });

    // 🔵 普通优先级
    QAction *normalAction = priorityMenu->addAction(tr("🔵 普通"));
    connect(normalAction, &QAction::triggered, [this, item]() {
        item->setData(static_cast<int>(TaskPriority::Normal), PriorityRole);
        updateItemStyle(item);
        saveToDefaultLocation();   // 🆕 自动保存
    });

    // ⚪ 低优先级
    QAction *lowAction = priorityMenu->addAction(tr("⚪ 低"));
    connect(lowAction, &QAction::triggered, [this, item]() {
        item->setData(static_cast<int>(TaskPriority::Low), PriorityRole);
        updateItemStyle(item);
        saveToDefaultLocation();   // 🆕 自动保存
    });

    menu.addSeparator();
        // 🆕 置顶（移到最前）
    QAction *moveTopAction = menu.addAction(tr("⬆️ 置顶"));
    connect(moveTopAction, &QAction::triggered, [this, index]() {
        if (index.row() == 0) return;   // 已经在顶部

        // 取出这一行
        QList<QStandardItem *> row = m_model->takeRow(index.row());
        // 插入到最前
        m_model->insertRow(0, row);
        saveToDefaultLocation();
        applyFilter();
    });

    // 🆕 置底
    QAction *moveBottomAction = menu.addAction(tr("⬇️ 置底"));
    connect(moveBottomAction, &QAction::triggered, [this, index]() {
        int lastRow = m_model->rowCount() - 1;
        if (index.row() == lastRow) return;   // 已经在底部

        QList<QStandardItem *> row = m_model->takeRow(index.row());
        m_model->appendRow(row);
        saveToDefaultLocation();
        applyFilter();
    });

    menu.addSeparator();
    // 🗑️ 删除
    QAction *deleteAction = menu.addAction(tr("🗑️ 删除"));
    connect(deleteAction, &QAction::triggered, [this, index]() {
        m_model->removeRow(index.row());
        updateStats();
        saveToDefaultLocation();   // 🆕 自动保存
    });

    menu.exec(m_listView->viewport()->mapToGlobal(pos));
}
// ===== 🆕 双击编辑 =====
void TaskListWidget::onItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    QStandardItem *item = m_model->itemFromIndex(index);
    if (!item) return;

    // 去掉优先级前缀
    QString currentText = item->text();
    if (currentText.startsWith("🔴 ") ||
        currentText.startsWith("🔵 ") ||
        currentText.startsWith("⚪ ")) {
        currentText = currentText.mid(3);
    }

    bool ok;
    QString newText = QInputDialog::getText(
        this,
        tr("编辑任务"),
        tr("任务内容:"),
        QLineEdit::Normal,
        currentText,
        &ok
    );

    if (ok && !newText.trimmed().isEmpty()) {
        item->setText(newText.trimmed());
        updateItemStyle(item);   // 重新加上前缀
        saveToDefaultLocation();
    }
}

// ===== 更新统计 =====
void TaskListWidget::updateStats()
{
    int total = totalCount();
    int done = completedCount();
    int pending = total - done;

    if (total == 0) {
        m_statsLabel->setText(tr("暂无任务"));
    } else {
        m_statsLabel->setText(
            tr("共 %1 个任务 | 待办: %2 | 已完成: %3")
                .arg(total).arg(pending).arg(done)
        );
    }

    emit taskCountChanged(total, done);
}

// ===== 统计 =====
int TaskListWidget::totalCount() const
{
    return m_model->rowCount();
}

int TaskListWidget::completedCount() const
{
    int count = 0;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (m_model->item(i)->checkState() == Qt::Checked) {
            count++;
        }
    }
    return count;
}

// ===== 🆕 获取所有任务（下一阶段持久化用）=====
QVector<Task> TaskListWidget::getAllTasks() const
{
    QVector<Task> tasks;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStandardItem *item = m_model->item(i);
        if (!item) continue;

        Task t;
        // 去掉优先级前缀
        QString text = item->text();
        if (text.startsWith("🔴 ") ||
            text.startsWith("🔵 ") ||
            text.startsWith("⚪ ")) {
            text = text.mid(3);
        }
        t.title = text;
        t.completed = (item->checkState() == Qt::Checked);
        t.priority = static_cast<TaskPriority>(
            item->data(PriorityRole).toInt()
        );
        tasks.append(t);
    }
    return tasks;
}

// ===== 🆕 加载任务列表（下一阶段持久化用）=====
void TaskListWidget::loadTasks(const QVector<Task> &tasks)
{
    m_model->clear();
    for (const Task &t : tasks) {
        addTaskItem(t);
    }
    updateStats();
}

// ============================================================
// 🆕 数据持久化
// ============================================================

// 获取默认存储路径
QString TaskListWidget::defaultTasksFilePath() const
{
    // 用 Qt 提供的"应用数据目录"（跨平台）
    // Windows: C:/Users/<user>/AppData/Roaming/<AppName>
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // 确保目录存在
    QDir().mkpath(dir);

    return dir + "/tasks.json";
}

// 保存到指定文件
void TaskListWidget::saveToFile(const QString &filePath)
{
    // 构造 JSON
    QJsonObject root;
    root["version"] = "1.0";

    QJsonArray tasksArray;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStandardItem *item = m_model->item(i);
        if (!item) continue;

        // 去掉优先级前缀
        QString text = item->text();
        if (text.startsWith("🔴 ") ||
            text.startsWith("🔵 ") ||
            text.startsWith("⚪ ")) {
            text = text.mid(3);
        }

        QJsonObject taskObj;
        taskObj["title"] = text;
        taskObj["completed"] = (item->checkState() == Qt::Checked);
        taskObj["priority"] = item->data(PriorityRole).toInt();
        // 时间用当前时间（简单处理，也可以加载时保留原时间）
        taskObj["createdTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);

        tasksArray.append(taskObj);
    }

    root["tasks"] = tasksArray;

    // 转成 JSON 字节
    QJsonDocument doc(root);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    // 写文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法保存任务文件:" << filePath
                   << "错误:" << file.errorString();
        return;
    }

    file.write(data);
    file.close();

    qDebug() << "任务已保存到:" << filePath
             << "共" << tasksArray.size() << "个任务";
}

// 从指定文件加载
void TaskListWidget::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        qDebug() << "任务文件不存在（首次使用）:" << filePath;
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开任务文件:" << filePath;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    // 解析 JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON 解析错误:" << parseError.errorString();
        return;
    }

    if (!doc.isObject()) return;

    QJsonObject root = doc.object();
    QJsonArray tasksArray = root["tasks"].toArray();

    // 清空当前列表
    m_model->clear();

    // 加载每个任务
    for (const QJsonValue &v : tasksArray) {
        QJsonObject obj = v.toObject();

        Task t;
        t.title = obj["title"].toString();
        t.completed = obj["completed"].toBool();
        t.priority = static_cast<TaskPriority>(obj["priority"].toInt());

        QString timeStr = obj["createdTime"].toString();
        if (!timeStr.isEmpty()) {
            t.createdTime = QDateTime::fromString(timeStr, Qt::ISODate);
        }

        addTaskItem(t);
    }

    qDebug() << "任务已加载:" << filePath
             << "共" << tasksArray.size() << "个任务";
}

// 保存到默认位置
void TaskListWidget::saveToDefaultLocation()
{
    saveToFile(defaultTasksFilePath());
}

// 从默认位置加载
void TaskListWidget::loadFromDefaultLocation()
{
    loadFromFile(defaultTasksFilePath());
}

// ============================================================
// 🆕 筛选功能
// ============================================================
void TaskListWidget::onFilterChanged(int index)
{
    Q_UNUSED(index);
    applyFilter();
}

void TaskListWidget::applyFilter()
{
    FilterMode mode = static_cast<FilterMode>(
        m_filterCombo->currentData().toInt()
    );

    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStandardItem *item = m_model->item(i);
        if (!item) continue;

        bool completed = (item->checkState() == Qt::Checked);
        bool visible = true;

        switch (mode) {
            case ShowAll:
                visible = true;
                break;
            case ShowPending:
                visible = !completed;
                break;
            case ShowCompleted:
                visible = completed;
                break;
        }

        m_listView->setRowHidden(i, !visible);
    }
}

// ============================================================
// 🆕 应用主题
// ============================================================
void TaskListWidget::applyTheme()
{
    const Theme &theme = ThemeManager::instance().currentTheme();

    // 判断是亮主题还是暗主题（根据背景色亮度）
    bool isDark = theme.editorBackground.lightness() < 128;

    // 定义颜色
    QString bgColor = theme.editorBackground.name();       // 背景
    QString fgColor = theme.editorForeground.name();       // 文字
    QString borderColor = isDark ? "#3F3F3F" : "#DDDDDD";  // 边框
    QString hoverBg = isDark ? "#2A2D2E" : "#F0F8FF";      // 悬停背景
    QString selectedBg = isDark ? "#094771" : "#E3F2FD";   // 选中背景
    QString itemBorder = isDark ? "#2F2F2F" : "#EEEEEE";   // 分隔线

    // 应用到 QListView
    QString listStyle = QString(
        "QListView { "
        "  border: 1px solid %1; "
        "  background: %2; "
        "  color: %3; "
        "  padding: 4px; "
        "} "
        "QListView::item { "
        "  padding: 6px; "
        "  color: %3; "
        "  border-bottom: 1px solid %4; "
        "} "
        "QListView::item:hover { "
        "  background: %5; "
        "} "
        "QListView::item:selected { "
        "  background: %6; "
        "  color: %3; "
        "}"
    ).arg(borderColor, bgColor, fgColor, itemBorder, hoverBg, selectedBg);

    m_listView->setStyleSheet(listStyle);

    // 应用到整个 widget 的背景
    QString widgetStyle = QString(
        "TaskListWidget { background: %1; color: %2; }"
        "QLabel { color: %2; }"
    ).arg(bgColor, fgColor);

    setStyleSheet(widgetStyle);

    // 重新应用每个任务项的颜色（防止旧颜色残留）
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStandardItem *item = m_model->item(i);
        if (item) {
            updateItemStyle(item);
        }
    }
}