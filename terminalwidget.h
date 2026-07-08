#ifndef TERMINALWIDGET_H
#define TERMINALWIDGET_H

#include <QWidget>

class QPlainTextEdit;
class QPushButton;
class QLabel;
class QProcess;
class QElapsedTimer;

class TerminalWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TerminalWidget(QWidget *parent = nullptr);
    ~TerminalWidget();

    // 运行指定文件（根据扩展名自动选择运行方式）
    void runFile(const QString &filePath);

    // 停止当前运行的进程
    void stopProcess();

    // 是否正在运行
    bool isRunning() const;

signals:
    // 运行状态变化
    void runningStateChanged(bool running);

private slots:
    void onStopClicked();
    void onClearClicked();
    void onProcessOutput();
    void onProcessError();
    void onProcessFinished(int exitCode, int exitStatus);
    void onProcessErrorOccurred(int error);

private:
    void setupUi();
    void applyTheme();

    // 追加输出到显示区（带颜色）
    void appendOutput(const QString &text, const QString &color = QString());

    // 根据文件扩展名生成命令
    struct RunCommand {
        QString program;              // 程序（如 python）
        QStringList arguments;        // 参数
        QString workingDir;           // 工作目录
        bool needsCompile;            // 是否需要编译（C++）
        QString compilerCommand;      // 编译命令
        QStringList compilerArgs;     // 编译参数
        QString executablePath;       // 编译后要运行的 exe
    };
    RunCommand getRunCommand(const QString &filePath);

    // UI
    QPlainTextEdit *m_outputDisplay;
    QPushButton *m_stopButton;
    QPushButton *m_clearButton;
    QLabel *m_statusLabel;

    // 进程
    QProcess *m_process;
    QElapsedTimer *m_timer;
    QString m_currentFile;
    QString m_pendingExecutable;   // 编译后要运行的 exe
};

#endif // TERMINALWIDGET_H