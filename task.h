#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>

// 任务的优先级
enum class TaskPriority {
    Low,      // 低
    Normal,   // 中
    High      // 高
};

// 一个任务
struct Task {
    QString title;               // 任务标题
    bool completed;              // 是否完成
    TaskPriority priority;       // 优先级
    QDateTime createdTime;       // 创建时间

    // 默认构造函数
    Task()
        : completed(false)
        , priority(TaskPriority::Normal)
        , createdTime(QDateTime::currentDateTime())
    {}

    // 快捷构造函数
    Task(const QString &t, TaskPriority p = TaskPriority::Normal)
        : title(t)
        , completed(false)
        , priority(p)
        , createdTime(QDateTime::currentDateTime())
    {}
};

#endif // TASK_H