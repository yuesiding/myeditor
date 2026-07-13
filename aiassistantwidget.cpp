#include "aiassistantwidget.h"
#include "thememanager.h"
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QKeyEvent>

AiAssistantWidget::AiAssistantWidget(QWidget *parent)
    : QWidget(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_waitingForReply(false)
{
    setupUi();

    // 网络响应
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &AiAssistantWidget::onNetworkReply);

    // 监听主题变化
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &AiAssistantWidget::applyTheme);

    applyTheme();
    refreshChatDisplay();
}

void AiAssistantWidget::setupUi()
{
    //标题 
    QLabel *titleLabel = new QLabel(tr("🤖AI助手"), this);
    titleLabel->setStyleSheet(
        "QLabel { padding: 6px; font-weight: bold; font-size: 13px; }"
    );

    //状态提示
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet(
        "QLabel { padding: 4px 6px; color: #888; font-size: 11px; }"
    );
    m_statusLabel->setText(tr("准备就绪"));

    //设置API Key
    m_configKeyButton = new QPushButton(tr("🔑 Set API Key"), this);
    connect(m_configKeyButton, &QPushButton::clicked,
            this, &AiAssistantWidget::onConfigKeyClicked);

    //对话显示
    m_chatDisplay = new QTextEdit(this);
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setPlaceholderText(tr("对话历史将显示在这里..."));

    //输入
    m_inputField = new QTextEdit(this);
    m_inputField->setPlaceholderText(tr("输入问题... (Ctrl+Enter 发送)"));
    m_inputField->setMaximumHeight(100);

    //拦截 Ctrl+Enter
    m_inputField->installEventFilter(this);

    //按键
    m_sendButton = new QPushButton(tr("📤 发送"), this);
    connect(m_sendButton, &QPushButton::clicked,
            this, &AiAssistantWidget::onSendClicked);

    m_clearButton = new QPushButton(tr("🗑️ 清空"), this);
    connect(m_clearButton, &QPushButton::clicked,
            this, &AiAssistantWidget::onClearClicked);

    //布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_sendButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(m_configKeyButton);
    mainLayout->addWidget(m_chatDisplay, 1);
    mainLayout->addWidget(m_inputField);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(m_statusLabel);

    setLayout(mainLayout);

    //更新按钮显示
    if (hasApiKey()) {
        m_configKeyButton->setText(tr("🔑 更换 API Key"));
    }
}

    //主题 
void AiAssistantWidget::applyTheme()
{
    const Theme &theme = ThemeManager::instance().currentTheme();
    bool isDark = theme.editorBackground.lightness() < 128;

    QString bgColor = theme.editorBackground.name();
    QString fgColor = theme.editorForeground.name();

    QString style = QString(
        "AiAssistantWidget { background: %1; color: %2; }"
        "QLabel { color: %2; }"
        "QTextEdit { "
        "  background: %1; color: %2; "
        "  border: 1px solid %3; padding: 4px; "
        "  font-family: 'Consolas', monospace; "
        "}"
    ).arg(bgColor, fgColor, isDark ? "#3F3F3F" : "#CCCCCC");

    setStyleSheet(style);

    refreshChatDisplay();
}

//事件过滤
bool AiAssistantWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_inputField && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if ((keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
            && (keyEvent->modifiers() & Qt::ControlModifier)) {
            onSendClicked();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

//API Key管理
QString AiAssistantWidget::apiKey() const
{
    QSettings settings("MyCompany", "CodeEditor");
    return settings.value("ai/apiKey").toString();
}

void AiAssistantWidget::setApiKey(const QString &key)
{
    QSettings settings("MyCompany", "CodeEditor");
    settings.setValue("ai/apiKey", key);
}

bool AiAssistantWidget::hasApiKey() const
{
    return !apiKey().isEmpty();
}

//配置API Key
void AiAssistantWidget::onConfigKeyClicked()
{
    QString currentKey = apiKey();
    QString displayKey;
    if (!currentKey.isEmpty()) {
        // 只显示前 8 位 + ***
        displayKey = currentKey.left(8) + "***";
    }

    bool ok;
    QString newKey = QInputDialog::getText(
        this,
        tr("配置API Key"),
        tr("请输入你的 API Key（以 sk- 开头）\n当前: %1")
            .arg(displayKey.isEmpty() ? tr("(未设置)") : displayKey),
        QLineEdit::Password,   // 隐藏显示
        "",
        &ok
    );

    if (!ok || newKey.trimmed().isEmpty()) return;

    if (!newKey.startsWith("sk-")) {
        QMessageBox::warning(this, tr("警告"),
                             tr("API Key 格式不正确，应以 'sk-' 开头"));
        return;
    }

    setApiKey(newKey.trimmed());
    QMessageBox::information(this, tr("成功"),
                             tr("API Key 已保存"));
    m_configKeyButton->setText(tr("🔑 更换 API Key"));
    m_statusLabel->setText(tr("API Key 已配置 ✓"));
}

//发送
void AiAssistantWidget::onSendClicked()
{
    QString question = m_inputField->toPlainText().trimmed();
    if (question.isEmpty()) return;

    if (!hasApiKey()) {
        QMessageBox::warning(this, tr("提示"),
                             tr("请先点击上方按钮设置 API Key"));
        return;
    }

    if (m_waitingForReply) {
        QMessageBox::information(this, tr("提示"),
                                  tr("正在等待上一条回复，请稍候..."));
        return;
    }

    m_inputField->clear();
    sendRequest(question);
}

//清空
void AiAssistantWidget::onClearClicked()
{
    auto ret = QMessageBox::question(
        this,
        tr("确认"),
        tr("确定要清空所有对话历史吗？"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (ret == QMessageBox::Yes) {
        m_messages.clear();
        refreshChatDisplay();
        m_statusLabel->setText(tr("对话已清空"));
    }
}

//从外部提问
void AiAssistantWidget::askQuestion(const QString &question)
{
    if (!hasApiKey()) {
        QMessageBox::warning(this, tr("提示"),
                             tr("请先设置 API Key"));
        return;
    }
    sendRequest(question);
}

//发送HTTP请求
void AiAssistantWidget::sendRequest(const QString &userMessage)
{
    //添加用户消息到历史
    appendMessage("user", userMessage);

    m_waitingForReply = true;
    m_sendButton->setEnabled(false);
    m_statusLabel->setText(tr("⏳ AI 思考中..."));

    //构造请求 URL
    QUrl url("https://api.deepseek.com/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey()).toUtf8());

    //构造请求body
    QJsonObject body;
    body["model"] = "deepseek-chat";
    body["temperature"] = 0.7;
    body["stream"] = false;

    //消息数组
    QJsonArray messages;

    //系统提示词
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = "你是一个专业的编程助手，请用简洁清晰的中文回答用户的问题。"
                          "如果涉及代码，用 Markdown 代码块格式化。";
    messages.append(systemMsg);

    //添加历史对话
    int startIdx = qMax(0, m_messages.size() - 10);
    for (int i = startIdx; i < m_messages.size(); ++i) {
        QJsonObject msg;
        msg["role"] = m_messages[i].role;
        msg["content"] = m_messages[i].content;
        messages.append(msg);
    }

    body["messages"] = messages;

    QJsonDocument doc(body);
    QByteArray data = doc.toJson();

    //发送POST请求
    m_networkManager->post(request, data);
}

//收到响应
void AiAssistantWidget::onNetworkReply(QNetworkReply *reply)
{
    m_waitingForReply = false;
    m_sendButton->setEnabled(true);

    if (reply->error() != QNetworkReply::NoError) {
        QString errorMsg = tr("网络错误: %1").arg(reply->errorString());
        appendMessage("assistant", errorMsg);
        m_statusLabel->setText(tr("❌ 请求失败"));
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        appendMessage("assistant", tr("解析响应失败: %1").arg(parseError.errorString()));
        m_statusLabel->setText(tr("❌ 解析失败"));
        return;
    }

    QJsonObject root = doc.object();

    //检查错误
    if (root.contains("error")) {
        QJsonObject err = root["error"].toObject();
        QString errMsg = err["message"].toString();
        appendMessage("assistant", tr("API 错误: %1").arg(errMsg));
        m_statusLabel->setText(tr("❌ API 错误"));
        return;
    }

    //提取回复
    QJsonArray choices = root["choices"].toArray();
    if (choices.isEmpty()) {
        appendMessage("assistant", tr("API 未返回内容"));
        m_statusLabel->setText(tr("❌ 空响应"));
        return;
    }

    QJsonObject firstChoice = choices[0].toObject();
    QJsonObject message = firstChoice["message"].toObject();
    QString content = message["content"].toString();

    //显示token使用量
    QJsonObject usage = root["usage"].toObject();
    int totalTokens = usage["total_tokens"].toInt();

    appendMessage("assistant", content);
    m_statusLabel->setText(tr("✓ 完成 (用 %1 tokens)").arg(totalTokens));
}

//添加消息
void AiAssistantWidget::appendMessage(const QString &role, const QString &content)
{
    ChatMessage msg;
    msg.role = role;
    msg.content = content;
    m_messages.append(msg);

    refreshChatDisplay();
}

// 刷新聊天
void AiAssistantWidget::refreshChatDisplay()
{
    const Theme &theme = ThemeManager::instance().currentTheme();
    bool isDark = theme.editorBackground.lightness() < 128;

    QString userColor = isDark ? "#4EC9B0" : "#0078D4";
    QString aiColor = isDark ? "#DCDCAA" : "#795E26";
    QString textColor = theme.editorForeground.name();
    QString bgColor = theme.editorBackground.name();
    QString hrColor = isDark ? "#3F3F3F" : "#DDD";

    QString html;
    html += QString("<style>"
                    "body { font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; "
                    "  color: %1; background: %5; font-size: 13px; }"
                    ".user-label { color: %2; font-weight: bold; margin-top: 8px; "
                    "  font-size: 14px; }"
                    ".ai-label { color: %3; font-weight: bold; margin-top: 8px; "
                    "  font-size: 14px; }"
                    ".content { margin-left: 8px; margin-bottom: 4px; white-space: pre-wrap; }"
                    "hr { border: 0; border-top: 1px solid %4; margin: 10px 0; }"
                    "</style>")
                .arg(textColor, userColor, aiColor, hrColor, bgColor);

    if (m_messages.isEmpty()) {
        html += "<p style='color:#888; text-align:center; padding:20px;'>"
                "💡 <b>使用提示</b><br><br>"
                "• 输入问题后点击「发送」<br>"
                "• 使用 <b>Ctrl+Enter</b> 快捷键发送<br>"
                "• 在代码编辑器里 <b>右键选中的代码</b> 可以让 AI 分析<br>"
                "</p>";
    } else {
        for (const ChatMessage &msg : m_messages) {
            if (msg.role == "user") {
                html += QString("<div class='user-label'>👤 你:</div>");
                QString userContent = msg.content.toHtmlEscaped();
                userContent.replace("\n", "<br>");
                html += QString("<div class='content'>%1</div>").arg(userContent);
            } else if (msg.role == "assistant") {
                html += QString("<div class='ai-label'>🤖 AI:</div>");
                // 🆕 简单显示：只转义 HTML + 换行，不做 Markdown 渲染
                QString aiContent = msg.content.toHtmlEscaped();
                aiContent.replace("\n", "<br>");
                html += QString("<div class='content'>%1</div>").arg(aiContent);
            }
            html += "<hr>";
        }
    }

    m_chatDisplay->setHtml(html);

    //滚动到底部
    QTextCursor cursor = m_chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_chatDisplay->setTextCursor(cursor);
}