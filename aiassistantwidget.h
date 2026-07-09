#ifndef AIASSISTANTWIDGET_H
#define AIASSISTANTWIDGET_H

#include <QWidget>
#include <QVector>

class QTextEdit;
class QLineEdit;
class QPushButton;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;

// 一条聊天消息
struct ChatMessage {
    QString role;      // "user" / "assistant" / "system"
    QString content;
};

class AiAssistantWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AiAssistantWidget(QWidget *parent = nullptr);

    // 从外部提问（比如右键菜单调用）
    void askQuestion(const QString &question);
        // 🆕 AI 生成话题建议（供思维导图使用）
    // 返回一个 signal 会异步发出结果
    void requestTopicSuggestions(const QString &parentTopic,
                                  const QStringList &existingChildren);

signals:
    // 🆕 话题建议返回
    void topicSuggestionsReady(const QStringList &topics);
    void topicSuggestionsError(const QString &error);
private slots:
    void onSendClicked();
    void onClearClicked();
    void onConfigKeyClicked();
    void onNetworkReply(QNetworkReply *reply);
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUi();
    void applyTheme();

    // API Key 管理
    QString apiKey() const;
    void setApiKey(const QString &key);
    bool hasApiKey() const;

    // 发送请求
    void sendRequest(const QString &userMessage);

    // 添加消息到对话历史
    void appendMessage(const QString &role, const QString &content);

    // 刷新聊天显示
    void refreshChatDisplay();

    // UI 控件
    QTextEdit *m_chatDisplay;      // 对话历史显示
    QTextEdit *m_inputField;       // 多行输入
    QPushButton *m_sendButton;
    QPushButton *m_clearButton;
    QPushButton *m_configKeyButton;
    QLabel *m_statusLabel;

    // 数据
    QVector<ChatMessage> m_messages;   // 对话历史
    QNetworkAccessManager *m_networkManager;
    bool m_waitingForReply;            // 是否在等待响应
        // 🆕 简单的 Markdown -> HTML 转换
    QString markdownToHtml(const QString &markdown) const;
        // 🆕 是否正在请求话题建议（不显示在聊天历史里）
    bool m_isRequestingTopics;
    QString m_topicParent;
};

#endif // AIASSISTANTWIDGET_H