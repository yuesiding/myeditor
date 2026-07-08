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
#include <QRegularExpression>

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
    // ===== 标题 =====
    QLabel *titleLabel = new QLabel(tr("🤖 AI 助手 (DeepSeek)"), this);
    titleLabel->setStyleSheet(
        "QLabel { padding: 6px; font-weight: bold; font-size: 13px; }"
    );

    // ===== 状态提示 =====
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet(
        "QLabel { padding: 4px 6px; color: #888; font-size: 11px; }"
    );
    m_statusLabel->setText(tr("准备就绪"));

    // ===== 设置 API Key 按钮 =====
    m_configKeyButton = new QPushButton(tr("🔑 设置 API Key"), this);
    connect(m_configKeyButton, &QPushButton::clicked,
            this, &AiAssistantWidget::onConfigKeyClicked);

    // ===== 对话显示区 =====
    m_chatDisplay = new QTextEdit(this);
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setPlaceholderText(tr("对话历史将显示在这里..."));

    // ===== 输入区 =====
    m_inputField = new QTextEdit(this);
    m_inputField->setPlaceholderText(tr("输入问题... (Ctrl+Enter 发送)"));
    m_inputField->setMaximumHeight(100);

    // 拦截 Ctrl+Enter
    m_inputField->installEventFilter(this);

    // ===== 按钮 =====
    m_sendButton = new QPushButton(tr("📤 发送"), this);
    connect(m_sendButton, &QPushButton::clicked,
            this, &AiAssistantWidget::onSendClicked);

    m_clearButton = new QPushButton(tr("🗑️ 清空"), this);
    connect(m_clearButton, &QPushButton::clicked,
            this, &AiAssistantWidget::onClearClicked);

    // ===== 布局 =====
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

    // 更新按钮显示（有 Key 就隐藏配置按钮变小）
    if (hasApiKey()) {
        m_configKeyButton->setText(tr("🔑 更换 API Key"));
    }
}

// ===== 主题 =====
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

// ===== 事件过滤（Ctrl+Enter 发送）=====
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

// ===== API Key 管理 =====
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

// ===== 配置 API Key =====
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
        tr("配置 DeepSeek API Key"),
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

// ===== 发送 =====
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

// ===== 清空 =====
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

// ===== 从外部提问 =====
void AiAssistantWidget::askQuestion(const QString &question)
{
    if (!hasApiKey()) {
        QMessageBox::warning(this, tr("提示"),
                             tr("请先设置 API Key"));
        return;
    }
    sendRequest(question);
}

// ===== 发送 HTTP 请求 =====
void AiAssistantWidget::sendRequest(const QString &userMessage)
{
    // 添加用户消息到历史
    appendMessage("user", userMessage);

    m_waitingForReply = true;
    m_sendButton->setEnabled(false);
    m_statusLabel->setText(tr("⏳ AI 思考中..."));

    // 构造请求 URL
    QUrl url("https://api.deepseek.com/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + apiKey()).toUtf8());

    // 构造请求 body
    QJsonObject body;
    body["model"] = "deepseek-chat";
    body["temperature"] = 0.7;
    body["stream"] = false;

    // 消息数组
    QJsonArray messages;

    // 系统提示词
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = "你是一个专业的编程助手，请用简洁清晰的中文回答用户的问题。"
                          "如果涉及代码，用 Markdown 代码块格式化。";
    messages.append(systemMsg);

    // 添加历史对话（保留最近 10 条，避免超 token）
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

    // 发送 POST 请求
    m_networkManager->post(request, data);
}

// ===== 收到响应 =====
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

    // 检查错误
    if (root.contains("error")) {
        QJsonObject err = root["error"].toObject();
        QString errMsg = err["message"].toString();
        appendMessage("assistant", tr("API 错误: %1").arg(errMsg));
        m_statusLabel->setText(tr("❌ API 错误"));
        return;
    }

    // 提取回复
    QJsonArray choices = root["choices"].toArray();
    if (choices.isEmpty()) {
        appendMessage("assistant", tr("API 未返回内容"));
        m_statusLabel->setText(tr("❌ 空响应"));
        return;
    }

    QJsonObject firstChoice = choices[0].toObject();
    QJsonObject message = firstChoice["message"].toObject();
    QString content = message["content"].toString();

    // 显示 token 使用量
    QJsonObject usage = root["usage"].toObject();
    int totalTokens = usage["total_tokens"].toInt();

    appendMessage("assistant", content);
    m_statusLabel->setText(tr("✓ 完成 (用 %1 tokens)").arg(totalTokens));
}

// ===== 添加消息 =====
void AiAssistantWidget::appendMessage(const QString &role, const QString &content)
{
    ChatMessage msg;
    msg.role = role;
    msg.content = content;
    m_messages.append(msg);

    refreshChatDisplay();
}

// ===== 刷新聊天显示 =====
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
                    ".content { margin-left: 8px; margin-bottom: 4px; }"
                    "hr { border: 0; border-top: 1px solid %4; margin: 10px 0; }"
                    "p { margin: 4px 0; line-height: 1.4; }"
                    "ul { margin: 4px 0 4px 0; padding-left: 24px; }"
                    "ol { margin: 4px 0 4px 0; padding-left: 24px; }"
                    "li { margin: 1px 0; padding: 0; }"
                    "h1, h2, h3 { margin: 8px 0 4px 0; }"
                    "code { padding: 1px 4px; }"
                    "</style>")
                .arg(textColor, userColor, aiColor, hrColor, bgColor);

    if (m_messages.isEmpty()) {
        html += "<p style='color:#888; text-align:center; padding:20px;'>"
                "💡 <b>使用提示</b><br><br>"
                "• 输入问题后点击「发送」<br>"
                "• 使用 <b>Ctrl+Enter</b> 快捷键发送<br>"
                "• 在代码编辑器里 <b>右键选中的代码</b> 可以让 AI 分析<br>"
                "• AI 回复支持 <b>Markdown 格式</b><br>"
                "</p>";
    } else {
        for (const ChatMessage &msg : m_messages) {
            if (msg.role == "user") {
                html += QString("<div class='user-label'>👤 你:</div>");
                // 用户消息只显示纯文本（转义 HTML）
                QString userContent = msg.content.toHtmlEscaped();
                userContent.replace("\n", "<br>");
                html += QString("<div class='content'>%1</div>").arg(userContent);
            } else if (msg.role == "assistant") {
                html += QString("<div class='ai-label'>🤖 AI:</div>");
                // AI 回复走 Markdown 渲染
                html += QString("<div class='content'>%1</div>")
                            .arg(markdownToHtml(msg.content));
            }
            html += "<hr>";
        }
    }

    m_chatDisplay->setHtml(html);

    // 滚动到底部
    QTextCursor cursor = m_chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_chatDisplay->setTextCursor(cursor);
}

// ============================================================
// 🆕 简单的 Markdown -> HTML 转换
// 支持：代码块、行内代码、加粗、斜体、标题、列表、链接
// ============================================================
QString AiAssistantWidget::markdownToHtml(const QString &markdown) const
{
    const Theme &theme = ThemeManager::instance().currentTheme();
    bool isDark = theme.editorBackground.lightness() < 128;

    // 代码块的背景和边框色
    QString codeBg = isDark ? "#1E1E1E" : "#F5F5F5";
    QString codeBorder = isDark ? "#3F3F3F" : "#DDDDDD";
    QString inlineCodeBg = isDark ? "#2D2D2D" : "#EEEEEE";
    QString codeColor = isDark ? "#D4D4D4" : "#333333";
    QString linkColor = isDark ? "#4EC9B0" : "#0078D4";
    QString headingColor = isDark ? "#569CD6" : "#0066CC";

    QString html = markdown;

    // ===== 步骤 1: 先提取代码块（避免它们被后面的替换影响）=====
    QRegularExpression codeBlockRegex(
        "```(\\w*)\\n([\\s\\S]*?)```",
        QRegularExpression::MultilineOption
    );

    QVector<QString> codeBlockPlaceholders;
    QVector<QString> codeBlockContents;

    QRegularExpressionMatchIterator it = codeBlockRegex.globalMatch(html);
    int codeBlockIndex = 0;

    QString processed;
    int lastEnd = 0;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        processed += html.mid(lastEnd, match.capturedStart() - lastEnd);

        QString lang = match.captured(1);
        QString code = match.captured(2);

        // 转义 HTML 特殊字符
        code = code.toHtmlEscaped();

        // 生成代码块 HTML
        QString codeHtml = QString(
            "<div style='background:%1; border:1px solid %2; "
            "border-radius:4px; padding:10px; margin:8px 0; "
            "font-family:Consolas,monospace; font-size:13px; "
            "color:%3; white-space:pre-wrap;'>"
            "<code>%4</code></div>"
        ).arg(codeBg, codeBorder, codeColor, code);

        // 用占位符替换（防止被后面的处理干扰）
        QString placeholder = QString("§§CODEBLOCK_%1§§").arg(codeBlockIndex);
        processed += placeholder;
        codeBlockPlaceholders.append(placeholder);
        codeBlockContents.append(codeHtml);

        codeBlockIndex++;
        lastEnd = match.capturedEnd();
    }
    processed += html.mid(lastEnd);
    html = processed;

    // ===== 步骤 2: 转义 HTML 特殊字符（除了占位符）=====
    // 因为占位符是安全的 §§，先保留它们
    html = html.toHtmlEscaped();

    // ===== 步骤 3: 处理行内代码 `code` =====
    QRegularExpression inlineCodeRegex("`([^`\\n]+)`");
    html.replace(inlineCodeRegex,
        QString("<code style='background:%1; padding:2px 6px; "
                "border-radius:3px; font-family:Consolas,monospace; "
                "font-size:13px; color:%2;'>\\1</code>")
            .arg(inlineCodeBg, codeColor));

    // ===== 步骤 4: 处理标题 =====
    // ### 标题
    QRegularExpression h3Regex("^### (.+)$", QRegularExpression::MultilineOption);
    html.replace(h3Regex,
        QString("<h3 style='color:%1; margin:10px 0 6px 0;'>\\1</h3>")
            .arg(headingColor));

    // ## 标题
    QRegularExpression h2Regex("^## (.+)$", QRegularExpression::MultilineOption);
    html.replace(h2Regex,
        QString("<h2 style='color:%1; margin:12px 0 8px 0;'>\\1</h2>")
            .arg(headingColor));

    // # 标题
    QRegularExpression h1Regex("^# (.+)$", QRegularExpression::MultilineOption);
    html.replace(h1Regex,
        QString("<h1 style='color:%1; margin:14px 0 10px 0;'>\\1</h1>")
            .arg(headingColor));

    // ===== 步骤 5: 处理加粗 **text** =====
    QRegularExpression boldRegex("\\*\\*([^\\*\\n]+)\\*\\*");
    html.replace(boldRegex, "<b>\\1</b>");

    // ===== 步骤 6: 处理斜体 *text* =====
    // 注意：要在加粗之后处理，避免误伤 **text**
    QRegularExpression italicRegex("(?<!\\*)\\*([^\\*\\n]+)\\*(?!\\*)");
    html.replace(italicRegex, "<i>\\1</i>");

    // ===== 步骤 7: 处理无序列表 =====
    QStringList lines = html.split('\n');
    bool inList = false;
    QStringList resultLines;

    for (const QString &line : lines) {
        // 匹配 - 或 * 开头的列表项
        QRegularExpression listRegex("^(\\s*)[-*]\\s+(.+)$");
        QRegularExpressionMatch match = listRegex.match(line);

        if (match.hasMatch()) {
            if (!inList) {
                resultLines.append("<ul style='margin:4px 0; padding-left:20px;'>");
                inList = true;
            }
            resultLines.append(QString("<li>%1</li>").arg(match.captured(2)));
        } else {
            if (inList) {
                resultLines.append("</ul>");
                inList = false;
            }
            resultLines.append(line);
        }
    }
    if (inList) {
        resultLines.append("</ul>");
    }
    html = resultLines.join('\n');

    // ===== 步骤 8: 处理链接 [text](url) =====
    QRegularExpression linkRegex("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
    html.replace(linkRegex,
        QString("<a href='\\2' style='color:%1;'>\\1</a>").arg(linkColor));

        // ===== 步骤 9: 清理列表相关的多余换行 =====
    // 列表元素之间的 <br> 会导致额外空白，要清理
    html.replace(QRegularExpression("<br>\\s*(<ul[^>]*>)"), "\\1");
    html.replace(QRegularExpression("(</ul>)\\s*<br>"), "\\1");
    html.replace(QRegularExpression("(</li>)\\s*<br>\\s*(<li)"), "\\1\\2");
    html.replace(QRegularExpression("(<ul[^>]*>)\\s*<br>"), "\\1");
    html.replace(QRegularExpression("<br>\\s*(</ul>)"), "\\1");

    // ===== 步骤 10: 处理换行 =====
    // 双换行 → 段落分隔
    // 单换行 → <br>
    html.replace("\n\n", "</p><p>");
    html.replace("\n", "<br>");

    // 清理段落里的空 <br>
    html.replace(QRegularExpression("<p[^>]*>\\s*<br>"), "<p style='margin:4px 0; line-height:1.4;'>");
    html.replace(QRegularExpression("<br>\\s*</p>"), "</p>");

    // 用 <p> 包起来
    html = "<p style='margin:4px 0; line-height:1.4;'>" + html + "</p>";

    // 清理空段落
    html.replace(QRegularExpression("<p[^>]*>\\s*</p>"), "");
    // ===== 步骤 10: 把代码块占位符替换回真实的 HTML =====
    for (int i = 0; i < codeBlockPlaceholders.size(); ++i) {
        html.replace(codeBlockPlaceholders[i], codeBlockContents[i]);
    }

    return html;
}