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

struct ChatMessage {
    QString role;     
    QString content;
};

class AiAssistantWidget : public QWidget
{
    Q_OBJECT//启用Qt信号槽与元对象系统

public:
    explicit AiAssistantWidget(QWidget *parent = nullptr);
    void askQuestion(const QString &question);//从外部提问
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
    //API Key管理
    QString apiKey() const;
    void setApiKey(const QString &key);
    bool hasApiKey() const;
    void sendRequest(const QString &userMessage);
    void appendMessage(const QString &role, const QString &content); //添加消息到对话历史
    void refreshChatDisplay(); // 刷新聊天显示
    //UI
    QTextEdit *m_chatDisplay;      //对话历史显示
    QTextEdit *m_inputField;       //多行输入
    QPushButton *m_sendButton;
    QPushButton *m_clearButton;
    QPushButton *m_configKeyButton;
    QLabel *m_statusLabel;
    //数据
    QVector<ChatMessage> m_messages;   //对话历史
    QNetworkAccessManager *m_networkManager;
    bool m_waitingForReply;            
};

#endif // AIASSISTANTWIDGET_H