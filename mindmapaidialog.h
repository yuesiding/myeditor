#ifndef MINDMAPAIDIALOG_H
#define MINDMAPAIDIALOG_H

#include <QDialog>
#include <QStringList>

class QListWidget;
class QPushButton;
class QLabel;
class QProgressBar;

// AI 建议对话框：展示 AI 生成的子话题，让用户勾选
class MindMapAiDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MindMapAiDialog(const QString &parentTopic, QWidget *parent = nullptr);

    // 设置 AI 建议的话题（AI 回复后调用）
    void setSuggestions(const QStringList &topics);

    // 获取用户勾选的话题
    QStringList selectedTopics() const;

    // 设置加载状态
    void setLoading(bool loading);

    // 设置错误信息
    void showError(const QString &error);

private slots:
    void onAcceptClicked();
    void onSelectAll();
    void onSelectNone();

private:
    void setupUi();
    void applyTheme();

    QString m_parentTopic;

    QLabel *m_titleLabel;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    QListWidget *m_suggestionList;
    QPushButton *m_selectAllButton;
    QPushButton *m_selectNoneButton;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // MINDMAPAIDIALOG_H