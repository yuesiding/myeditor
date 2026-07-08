#include "terminalwidget.h"
#include "thememanager.h"

#include <QPlainTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QElapsedTimer>
#include <QDateTime>

TerminalWidget::TerminalWidget(QWidget *parent)
    : QWidget(parent)
    , m_process(new QProcess(this))
    , m_timer(new QElapsedTimer())
{
    setupUi();

    // 进程信号
    connect(m_process, &QProcess::readyReadStandardOutput,
            this, &TerminalWidget::onProcessOutput);
    connect(m_process, &QProcess::readyReadStandardError,
            this, &TerminalWidget::onProcessError);
    connect(m_process,
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus exitStatus) {
                onProcessFinished(exitCode, static_cast<int>(exitStatus));
            });
    connect(m_process,
            QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
            this, [this](QProcess::ProcessError err) {
                onProcessErrorOccurred(static_cast<int>(err));
            });

    // 主题
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &TerminalWidget::applyTheme);
    applyTheme();
}

TerminalWidget::~TerminalWidget()
{
    if (m_process && m_process->state() != QProcess::NotRunning) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
    delete m_timer;
}

void TerminalWidget::setupUi()
{
    // ===== 顶部工具条 =====
    m_stopButton = new QPushButton(tr("⏹️ 停止"), this);
    m_stopButton->setEnabled(false);
    connect(m_stopButton, &QPushButton::clicked,
            this, &TerminalWidget::onStopClicked);

    m_clearButton = new QPushButton(tr("🗑️ 清空"), this);
    connect(m_clearButton, &QPushButton::clicked,
            this, &TerminalWidget::onClearClicked);

    m_statusLabel = new QLabel(tr("准备就绪"), this);

    QHBoxLayout *toolbarLayout = new QHBoxLayout;
    toolbarLayout->addWidget(m_stopButton);
    toolbarLayout->addWidget(m_clearButton);
    toolbarLayout->addStretch();
    toolbarLayout->addWidget(m_statusLabel);
    toolbarLayout->setContentsMargins(4, 2, 4, 2);

    // ===== 输出区 =====
    m_outputDisplay = new QPlainTextEdit(this);
    m_outputDisplay->setReadOnly(true);
    m_outputDisplay->setPlaceholderText(tr("程序运行输出将显示在这里..."));

    QFont font("Consolas", 10);
    font.setStyleHint(QFont::Monospace);
    m_outputDisplay->setFont(font);

    // ===== 布局 =====
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addLayout(toolbarLayout);
    mainLayout->addWidget(m_outputDisplay, 1);

    setLayout(mainLayout);
}

void TerminalWidget::applyTheme()
{
    const Theme &theme = ThemeManager::instance().currentTheme();
    bool isDark = theme.editorBackground.lightness() < 128;

    QString bgColor = isDark ? "#1E1E1E" : "#FAFAFA";
    QString fgColor = isDark ? "#D4D4D4" : "#333333";

    m_outputDisplay->setStyleSheet(QString(
        "QPlainTextEdit { "
        "  background: %1; color: %2; "
        "  border: none; padding: 4px; "
        "  font-family: 'Consolas', monospace; "
        "}"
    ).arg(bgColor, fgColor));

    m_statusLabel->setStyleSheet(
        QString("QLabel { padding: 2px 6px; color: #888; }")
    );
}

// ===== 运行文件 =====
void TerminalWidget::runFile(const QString &filePath)
{
    if (isRunning()) {
        appendOutput(tr("⚠️ 已有程序在运行，请先停止！\n"), "#E74C3C");
        return;
    }

    if (filePath.isEmpty()) {
        appendOutput(tr("⚠️ 请先保存文件！\n"), "#E74C3C");
        return;
    }

    QFileInfo fi(filePath);
    if (!fi.exists()) {
        appendOutput(tr("⚠️ 文件不存在: %1\n").arg(filePath), "#E74C3C");
        return;
    }

    m_currentFile = filePath;

    // 清屏，加分隔
    appendOutput(QString("\n"), QString());
    appendOutput(tr("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"), "#888");
    appendOutput(tr("▶ 运行: %1\n").arg(fi.fileName()), "#4EC9B0");
    appendOutput(tr("  时间: %1\n")
                     .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")),
                 "#888");
    appendOutput(tr("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"), "#888");

    // 获取运行命令
    RunCommand cmd = getRunCommand(filePath);

    if (cmd.program.isEmpty() && cmd.compilerCommand.isEmpty()) {
        appendOutput(tr("⚠️ 不支持的文件类型: .%1\n").arg(fi.suffix()), "#E74C3C");
        appendOutput(tr("支持的类型：.py .js .cpp .c .bat\n"), "#888");
        return;
    }

    m_timer->start();

    if (cmd.needsCompile) {
        // C/C++：先编译再运行
        appendOutput(tr("🔨 编译中...\n"), "#569CD6");

        m_pendingExecutable = cmd.executablePath;

        m_process->setWorkingDirectory(cmd.workingDir);
        m_process->start(cmd.compilerCommand, cmd.compilerArgs);
    } else {
        // 直接运行
        appendOutput(tr("🚀 执行: %1 %2\n\n")
                         .arg(cmd.program, cmd.arguments.join(" ")),
                     "#569CD6");

        m_pendingExecutable.clear();
        m_process->setWorkingDirectory(cmd.workingDir);
        m_process->start(cmd.program, cmd.arguments);
    }

    m_stopButton->setEnabled(true);
    m_statusLabel->setText(tr("运行中..."));
    emit runningStateChanged(true);
}

// ===== 根据扩展名生成运行命令 =====
TerminalWidget::RunCommand TerminalWidget::getRunCommand(const QString &filePath)
{
    RunCommand cmd;
    cmd.needsCompile = false;

    QFileInfo fi(filePath);
    QString suffix = fi.suffix().toLower();
    cmd.workingDir = fi.absolutePath();

    if (suffix == "py" || suffix == "pyw") {
        // 🆕 用 python 或 python3，都试试
        cmd.program = "python";
        cmd.arguments << filePath;
    }
    else if (suffix == "js") {
        cmd.program = "node";
        cmd.arguments << filePath;
    }
    else if (suffix == "cpp" || suffix == "cc" || suffix == "cxx") {
        cmd.needsCompile = true;
        // 🆕 用 MinGW 的绝对路径（你之前编译 CodeEditor 用的这个）
        cmd.compilerCommand = "H:/Qt/Tools/mingw730_64/bin/g++.exe";
        cmd.executablePath = fi.absolutePath() + "/" + fi.baseName() + ".exe";
        cmd.compilerArgs << filePath << "-o" << cmd.executablePath
                         << "-std=c++17" << "-O2";
    }
    else if (suffix == "c") {
        cmd.needsCompile = true;
        // 🆕 用 MinGW 的绝对路径
        cmd.compilerCommand = "H:/Qt/Tools/mingw730_64/bin/gcc.exe";
        cmd.executablePath = fi.absolutePath() + "/" + fi.baseName() + ".exe";
        cmd.compilerArgs << filePath << "-o" << cmd.executablePath;
    }
    else if (suffix == "bat" || suffix == "cmd") {
        cmd.program = "cmd";
        cmd.arguments << "/c" << filePath;
    }

    return cmd;
}
// ===== 进程输出 =====
void TerminalWidget::onProcessOutput()
{
    QByteArray data = m_process->readAllStandardOutput();
    QString text = QString::fromLocal8Bit(data);
    appendOutput(text);
}

void TerminalWidget::onProcessError()
{
    QByteArray data = m_process->readAllStandardError();
    QString text = QString::fromLocal8Bit(data);
    appendOutput(text, "#E74C3C");   // 错误输出用红色
}

// ===== 进程结束 =====
void TerminalWidget::onProcessFinished(int exitCode, int exitStatus)
{
    Q_UNUSED(exitStatus);

    qint64 elapsed = m_timer->elapsed();
    double seconds = elapsed / 1000.0;

    // 如果是编译，编译完自动运行 exe
    if (!m_pendingExecutable.isEmpty() && exitCode == 0) {
        QString exe = m_pendingExecutable;
        m_pendingExecutable.clear();

        appendOutput(tr("✅ 编译成功 (用时 %1 秒)\n\n").arg(seconds, 0, 'f', 2), "#4EC9B0");
        appendOutput(tr("🚀 执行: %1\n\n").arg(QFileInfo(exe).fileName()), "#569CD6");

        m_timer->restart();
        m_process->setWorkingDirectory(QFileInfo(exe).absolutePath());
        m_process->start(exe);
        return;
    }

    // 编译失败
    if (!m_pendingExecutable.isEmpty() && exitCode != 0) {
        m_pendingExecutable.clear();
        appendOutput(tr("\n❌ 编译失败 (退出码 %1)\n").arg(exitCode), "#E74C3C");
        m_stopButton->setEnabled(false);
        m_statusLabel->setText(tr("编译失败"));
        emit runningStateChanged(false);
        return;
    }

    // 正常运行结束
    appendOutput(tr("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"), "#888");
    if (exitCode == 0) {
        appendOutput(tr("✅ 程序运行完成，退出码: 0，用时 %1 秒\n")
                         .arg(seconds, 0, 'f', 2), "#4EC9B0");
    } else {
        appendOutput(tr("❌ 程序异常退出，退出码: %1，用时 %2 秒\n")
                         .arg(exitCode).arg(seconds, 0, 'f', 2), "#E74C3C");
    }

    m_stopButton->setEnabled(false);
    m_statusLabel->setText(tr("已完成"));
    emit runningStateChanged(false);
}

// ===== 进程错误 =====
void TerminalWidget::onProcessErrorOccurred(int error)
{
    QProcess::ProcessError err = static_cast<QProcess::ProcessError>(error);

    QString msg;
    switch (err) {
        case QProcess::FailedToStart:
            msg = tr("❌ 无法启动程序！可能是没有安装对应的解释器/编译器\n"
                     "   请确保已安装 python / node / g++ 等\n");
            break;
        case QProcess::Crashed:
            msg = tr("💥 程序崩溃了\n");
            break;
        case QProcess::Timedout:
            msg = tr("⏱️ 运行超时\n");
            break;
        default:
            msg = tr("❌ 进程错误\n");
    }

    appendOutput(msg, "#E74C3C");
    m_stopButton->setEnabled(false);
    m_statusLabel->setText(tr("错误"));
    emit runningStateChanged(false);
}

// ===== 停止按钮 =====
void TerminalWidget::onStopClicked()
{
    stopProcess();
}

void TerminalWidget::stopProcess()
{
    if (isRunning()) {
        m_process->kill();
        m_process->waitForFinished(1000);
        appendOutput(tr("\n⏹️ 用户中断了程序\n"), "#F39C12");
        m_stopButton->setEnabled(false);
        m_statusLabel->setText(tr("已中断"));
        emit runningStateChanged(false);
    }
}

// ===== 清空按钮 =====
void TerminalWidget::onClearClicked()
{
    m_outputDisplay->clear();
    m_statusLabel->setText(tr("准备就绪"));
}

bool TerminalWidget::isRunning() const
{
    return m_process->state() != QProcess::NotRunning;
}

// ===== 追加输出（带颜色）=====
void TerminalWidget::appendOutput(const QString &text, const QString &color)
{
    QTextCursor cursor = m_outputDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);

    QTextCharFormat format;
    if (!color.isEmpty()) {
        format.setForeground(QColor(color));
    } else {
        const Theme &theme = ThemeManager::instance().currentTheme();
        format.setForeground(theme.editorForeground);
    }

    cursor.setCharFormat(format);
    cursor.insertText(text);

    // 自动滚动到底部
    m_outputDisplay->setTextCursor(cursor);
    m_outputDisplay->ensureCursorVisible();
}