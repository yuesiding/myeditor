#ifndef TASKLISTWIDGET_H
#define TASKLISTWIDGET_H

#include <QWidget>
#include <QVector>
#include "task.h"

class QListView;
class QStandardItemModel;
class QStandardItem;
class QLineEdit;
class QPushButton;
class QLabel;
class QMenu;
class QModelIndex;
class QPoint;
class QComboBox;

class TaskListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TaskListWidget(QWidget *parent = nullptr);

    // 统计
    int totalCount() const;
    int completedCount() const;

    // 🆕 获取所有任务（用于持久化，下一阶段用）
    QVector<Task> getAllTasks() const;

    // 🆕 加载任务列表（用于持久化，下一阶段用）
    void loadTasks(const QVector<Task> &tasks);
        // 🆕 保存/加载 JSON 文件
    void saveToFile(const QString &filePath);
    void loadFromFile(const QString &filePath);

    // 🆕 使用默认位置保存/加载（简化调用）
    void saveToDefaultLocation();
    void loadFromDefaultLocation();
        // 筛选枚举
    enum FilterMode {
        ShowAll,
        ShowPending,
        ShowCompleted
    };
signals:
    void taskCountChanged(int total, int completed);

private slots:
    void onAddTaskClicked();
    void onRemoveCompletedClicked();

    // 🆕 复选框状态改变
    void onItemChanged(QStandardItem *item);

    // 🆕 右键菜单
    void onCustomContextMenu(const QPoint &pos);

    // 🆕 双击编辑
    void onItemDoubleClicked(const QModelIndex &index);
        // 🆕 筛选
    void onFilterChanged(int index);
        // 🆕 应用当前主题
    void applyTheme();
private:
    void setupUi();

    // 添加一个任务项到列表
    void addTaskItem(const Task &task);

    // 🆕 更新统计标签
    void updateStats();

    // 🆕 更新单个项目的显示样式（根据完成状态和优先级）
    void updateItemStyle(QStandardItem *item);

    // 🆕 从 QStandardItem 反查 Task 数据
    // 我们把优先级存到 UserRole
    // 存优先级的角色常量
    static constexpr int PriorityRole = Qt::UserRole + 1;

    // UI 控件
    QLabel *m_titleLabel;
    QLineEdit *m_inputField;
    QPushButton *m_addButton;
    QListView *m_listView;
    QStandardItemModel *m_model;
    QPushButton *m_removeCompletedButton;
    QLabel *m_statsLabel;
        // 🆕 获取默认存储路径
    QString defaultTasksFilePath() const;
        // 🆕 筛选下拉框
    QComboBox *m_filterCombo;

    void applyFilter();// 🆕 应用筛选（隐藏/显示项）
};

#endif // TASKLISTWIDGET_H