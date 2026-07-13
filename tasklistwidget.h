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

class TaskListWidget:public QWidget
{
    Q_OBJECT

public:
    explicit TaskListWidget(QWidget *parent=nullptr);

    int totalCount() const;
    int completedCount() const;
    QVector<Task> getAllTasks() const;
    void loadTasks(const QVector<Task>&tasks);
    void saveToFile(const QString &filePath);
    void loadFromFile(const QString &filePath);
    void saveToDefaultLocation();
    void loadFromDefaultLocation();
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
    void onItemChanged(QStandardItem *item);
    void onCustomContextMenu(const QPoint &pos);
    void onItemDoubleClicked(const QModelIndex &index);
    void onFilterChanged(int index);
    void applyTheme();
private:
    void setupUi();
    void addTaskItem(const Task &task);
    void updateStats();
    void updateItemStyle(QStandardItem *item);
    static constexpr int PriorityRole=Qt::UserRole+1;

    QLabel *m_titleLabel;
    QLineEdit *m_inputField;
    QPushButton *m_addButton;
    QListView *m_listView;
    QStandardItemModel *m_model;
    QPushButton *m_removeCompletedButton;
    QLabel *m_statsLabel;
       
    QString defaultTasksFilePath() const;
    QComboBox *m_filterCombo;
    void applyFilter();
};

#endif // TASKLISTWIDGET_H