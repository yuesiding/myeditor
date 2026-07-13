#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDateTime>

enum class TaskPriority {
    Low,      
    Normal,   
    High      
};

struct Task {
    QString title;               
    bool completed;              
    TaskPriority priority;       
    QDateTime createdTime;       
    Task(): completed(false), priority(TaskPriority::Normal), createdTime(QDateTime::currentDateTime()){}
    Task(const QString &t, TaskPriority p = TaskPriority::Normal):title(t),completed(false),priority(p),createdTime(QDateTime::currentDateTime()){}
};
#endif // TASK_H