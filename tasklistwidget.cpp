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

TaskListWidget::TaskListWidget(QWidget *parent):QWidget(parent){
    setupUi();
    loadFromDefaultLocation();
    if (m_model->rowCount() == 0){
        addTaskItem(Task("欢迎使用任务清单！点击复选框标记完成", TaskPriority::Normal));
        addTaskItem(Task("试试右键任务查看更多操作", TaskPriority::Low));
    }
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &TaskListWidget::applyTheme);
    applyTheme();
    updateStats();
}

void TaskListWidget::setupUi()
{
    m_titleLabel = new QLabel(tr("📋 任务清单"), this);
    m_titleLabel->setStyleSheet(
        "QLabel { padding: 6px; font-weight: bold; font-size: 13px; }"
    );
    m_inputField = new QLineEdit(this);
    m_inputField->setPlaceholderText(tr("输入新任务，回车添加..."));

    m_addButton = new QPushButton(tr("➕ 添加"), this);
    connect(m_addButton, &QPushButton::clicked,this,&TaskListWidget::onAddTaskClicked);
    connect(m_inputField, &QLineEdit::returnPressed,this, &TaskListWidget::onAddTaskClicked);
    m_filterCombo = new QComboBox(this);
    m_filterCombo->addItem(tr("📋 全部"), ShowAll);
    m_filterCombo->addItem(tr("⏳ 未完成"), ShowPending);
    m_filterCombo->addItem(tr("✅ 已完成"), ShowCompleted);
    connect(m_filterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),this, &TaskListWidget::onFilterChanged);
    m_model = new QStandardItemModel(this);
    m_listView = new QListView(this);
    m_listView->setModel(m_model);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);
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
    connect(m_model, &QStandardItemModel::itemChanged,this, &TaskListWidget::onItemChanged);
    m_listView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_listView, &QListView::customContextMenuRequested,this, &TaskListWidget::onCustomContextMenu);
    connect(m_listView, &QListView::doubleClicked,this, &TaskListWidget::onItemDoubleClicked);
    m_removeCompletedButton = new QPushButton(tr("🗑️ 删除已完成"), this);
    connect(m_removeCompletedButton, &QPushButton::clicked,this, &TaskListWidget::onRemoveCompletedClicked);
    m_statsLabel = new QLabel(this);
    m_statsLabel->setStyleSheet("QLabel { padding: 4px; color: #666; font-size: 11px; }");

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
void TaskListWidget::addTaskItem(const Task &task){
    QStandardItem *item = new QStandardItem();
    item->setText(task.title);
    item->setCheckable(true);
    item->setCheckState(task.completed ? Qt::Checked : Qt::Unchecked);
    item->setData(static_cast<int>(task.priority), PriorityRole);
    m_model->appendRow(item);
    updateItemStyle(item);
}
void TaskListWidget::updateItemStyle(QStandardItem *item){
    if (!item) return;
    bool completed=(item->checkState() == Qt::Checked);
    int priorityInt=item->data(PriorityRole).toInt();
    TaskPriority priority = static_cast<TaskPriority>(priorityInt);
    QFont f = item->font();
    f.setStrikeOut(completed);
    item->setFont(f);
    const Theme &theme=ThemeManager::instance().currentTheme();
    bool isDark = theme.editorBackground.lightness()<128;
    if (completed) {
        item->setForeground(QBrush(isDark ? QColor("#666") : QColor("#999")));
    }else{
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
    QString prefix;
    switch (priority) {
        case TaskPriority::High:prefix = "🔴 "; break;
        case TaskPriority::Normal:prefix = "🔵 "; break;
        case TaskPriority::Low:prefix = "⚪ "; break;
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
    saveToDefaultLocation();  
}

void TaskListWidget::onRemoveCompletedClicked()
{
    
    int completedNum = completedCount();
    if (completedNum == 0) {
        QMessageBox::information(this, tr("提示"), tr("没有已完成的任务"));
        return;
    }

    auto ret = QMessageBox::question(this,tr("确认删除"),tr("确定要删除 %1 个已完成的任务吗？").arg(completedNum),QMessageBox::Yes | QMessageBox::No);
    if (ret != QMessageBox::Yes) return;
    for (int i=m_model->rowCount()-1;i>=0;--i){
        QStandardItem *item = m_model->item(i);
        if (item&&item->checkState()==Qt::Checked) {
            m_model->removeRow(i);
        }
    }
    updateStats();
     saveToDefaultLocation();  
}

void TaskListWidget::onItemChanged(QStandardItem *item)
{
    updateItemStyle(item);
    updateStats();
    saveToDefaultLocation(); 
    applyFilter();
}
void TaskListWidget::onCustomContextMenu(const QPoint &pos)
{
    QModelIndex index = m_listView->indexAt(pos);
    if (!index.isValid()) return;

    QStandardItem *item = m_model->itemFromIndex(index);
    if (!item) return;

    QMenu menu(this);
    QAction *editAction = menu.addAction(tr("✏️ 编辑"));
    connect(editAction, &QAction::triggered, [this, index]() {
        onItemDoubleClicked(index);
    });

    QAction *copyAction = menu.addAction(tr("📋 复制"));
    connect(copyAction, &QAction::triggered, [this, item]() {
        QString text = item->text();
        if (text.startsWith("🔴 ") || text.startsWith("🔵 ") ||text.startsWith("⚪ ")) {
            text = text.mid(3);
        }
        QApplication::clipboard()->setText(text);
    });
    menu.addSeparator();
    QMenu *priorityMenu = menu.addMenu(tr("优先级"));

    QAction *highAction = priorityMenu->addAction(tr("🔴 高"));
    connect(highAction, &QAction::triggered, [this, item]() {
        item->setData(static_cast<int>(TaskPriority::High), PriorityRole);
        updateItemStyle(item);
        saveToDefaultLocation();   
    });
    QAction *normalAction = priorityMenu->addAction(tr("🔵 普通"));
    connect(normalAction, &QAction::triggered, [this, item]() {
        item->setData(static_cast<int>(TaskPriority::Normal), PriorityRole);
        updateItemStyle(item);
        saveToDefaultLocation();  
    });
    QAction *lowAction = priorityMenu->addAction(tr("⚪ 低"));
    connect(lowAction, &QAction::triggered, [this, item]() {
        item->setData(static_cast<int>(TaskPriority::Low), PriorityRole);
        updateItemStyle(item);
        saveToDefaultLocation();  
    });

    menu.addSeparator();
    QAction *moveTopAction = menu.addAction(tr("⬆️ 置顶"));
    connect(moveTopAction, &QAction::triggered, [this, index]() {
        if (index.row() == 0) return;   // 已经在顶部
        QList<QStandardItem *> row = m_model->takeRow(index.row());
        m_model->insertRow(0, row);
        saveToDefaultLocation();
        applyFilter();
    });

    QAction *moveBottomAction=menu.addAction(tr("⬇️ 置底"));
    connect(moveBottomAction, &QAction::triggered, [this, index]() {
        int lastRow=m_model->rowCount()-1;
        if (index.row() == lastRow) return;  
        QList<QStandardItem *> row = m_model->takeRow(index.row());
        m_model->appendRow(row);
        saveToDefaultLocation();
        applyFilter();
    });
    menu.addSeparator();
    QAction *deleteAction = menu.addAction(tr("🗑️ 删除"));
    connect(deleteAction, &QAction::triggered, [this, index]() {
        m_model->removeRow(index.row());
        updateStats();
        saveToDefaultLocation();  
    });

    menu.exec(m_listView->viewport()->mapToGlobal(pos));
}

void TaskListWidget::onItemDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;
    QStandardItem *item = m_model->itemFromIndex(index);
    if (!item) return;
    QString currentText = item->text();
    if (currentText.startsWith("🔴 ") ||currentText.startsWith("🔵 ") || currentText.startsWith("⚪ ")) {
        currentText = currentText.mid(3);
    }
    bool ok;
    QString newText = QInputDialog::getText(this,tr("编辑任务"),tr("任务内容:"),QLineEdit::Normal,currentText,&ok);
    if (ok && !newText.trimmed().isEmpty()) {
        item->setText(newText.trimmed());
        updateItemStyle(item);  
        saveToDefaultLocation();
    }
}

void TaskListWidget::updateStats()
{
    int total = totalCount();
    int done = completedCount();
    int pending = total - done;

    if (total == 0) {
        m_statsLabel->setText(tr("暂无任务"));
    } else {
        m_statsLabel->setText(tr("共 %1 个任务 | 待办: %2 | 已完成: %3").arg(total).arg(pending).arg(done));
    }
    emit taskCountChanged(total, done);
}


int TaskListWidget::totalCount() const{
    return m_model->rowCount();
}

int TaskListWidget::completedCount() const{
    int count = 0;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        if (m_model->item(i)->checkState() == Qt::Checked) {
            count++;
        }
    }
    return count;
}
QVector<Task> TaskListWidget::getAllTasks() const
{
    QVector<Task> tasks;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStandardItem *item = m_model->item(i);
        if (!item) continue;
        Task t;
       
        QString text = item->text();
        if (text.startsWith("🔴 ") ||text.startsWith("🔵 ") ||text.startsWith("⚪ ")){
            text = text.mid(3);
        }
        t.title=text;
        t.completed=(item->checkState() == Qt::Checked);
        t.priority=static_cast<TaskPriority>(
            item->data(PriorityRole).toInt()
        );
        tasks.append(t);
    }
    return tasks;
}
void TaskListWidget::loadTasks(const QVector<Task> &tasks)
{
    m_model->clear();
    for (const Task &t : tasks) {
        addTaskItem(t);
    }
    updateStats();
}

QString TaskListWidget::defaultTasksFilePath() const
{
    // Windows: C:/Users/<user>/AppData/Roaming/<AppName>
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/tasks.json";
}

void TaskListWidget::saveToFile(const QString &filePath)
{
    QJsonObject root;
    root["version"] = "1.0";

    QJsonArray tasksArray;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QStandardItem *item = m_model->item(i);
        if (!item) continue;
        QString text = item->text();
        if (text.startsWith("🔴 ")||text.startsWith("🔵 ")||text.startsWith("⚪ ")) {
            text = text.mid(3);
        }
        QJsonObject taskObj;
        taskObj["title"] = text;
        taskObj["completed"] = (item->checkState() == Qt::Checked);
        taskObj["priority"] = item->data(PriorityRole).toInt();
        taskObj["createdTime"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        tasksArray.append(taskObj);
    }
    root["tasks"] = tasksArray;
    QJsonDocument doc(root);
    QByteArray data = doc.toJson(QJsonDocument::Indented);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "无法保存任务文件:" << filePath<< "错误:" << file.errorString();
        return;
    }
    file.write(data);
    file.close();
    qDebug() << "任务已保存到:" << filePath
             << "共" << tasksArray.size() << "个任务";
}
void TaskListWidget::loadFromFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        qDebug()<<"任务文件不存在（首次使用）:"<<filePath;
        return;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning()<<"无法打开任务文件:"<<filePath;
        return;
    }
    QByteArray data = file.readAll();
    file.close();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() <<"JSON 解析错误:"<< parseError.errorString();
        return;
    }
    if (!doc.isObject()) return;
    QJsonObject root = doc.object();
    QJsonArray tasksArray = root["tasks"].toArray();

    m_model->clear();

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
    qDebug() << "任务已加载:" << filePath<< "共" << tasksArray.size() << "个任务";
}

void TaskListWidget::saveToDefaultLocation(){
    saveToFile(defaultTasksFilePath());
}
void TaskListWidget::loadFromDefaultLocation(){
    loadFromFile(defaultTasksFilePath());
}

void TaskListWidget::onFilterChanged(int index)
{
    Q_UNUSED(index);
    applyFilter();
}

void TaskListWidget::applyFilter()
{
    FilterMode mode = static_cast<FilterMode>(m_filterCombo->currentData().toInt());

    for (int i=0;i<m_model->rowCount();++i){
        QStandardItem *item=m_model->item(i);
        if (!item) continue;
        bool completed = (item->checkState() == Qt::Checked);
        bool visible = true;
        switch (mode){
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
void TaskListWidget::applyTheme()
{
    const Theme &theme = ThemeManager::instance().currentTheme();
    bool isDark = theme.editorBackground.lightness() < 128;

    QString bgColor = theme.editorBackground.name();       
    QString fgColor = theme.editorForeground.name();      
    QString borderColor = isDark ? "#3F3F3F" : "#DDDDDD";  
    QString hoverBg = isDark ? "#2A2D2E" : "#F0F8FF";      
    QString selectedBg = isDark ? "#094771" : "#E3F2FD";   
    QString itemBorder = isDark ? "#2F2F2F" : "#EEEEEE";  
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
    QString widgetStyle = QString(
        "TaskListWidget { background: %1; color: %2; }"
        "QLabel { color: %2; }"
    ).arg(bgColor, fgColor);

    setStyleSheet(widgetStyle);
    for (int i=0;i<m_model->rowCount();++i) {
        QStandardItem *item = m_model->item(i);
        if(item){
            updateItemStyle(item);
        }
    }
}