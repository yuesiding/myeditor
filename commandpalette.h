#ifndef COMMANDPALETTE_H
#define COMMANDPALETTE_H

#include <QDialog>
#include <QVector>
#include <functional>

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QKeyEvent;

// 一个命令
struct Command {
    QString icon;
    QString title;
    QString shortcut;
    std::function<void()> action;
};

class CommandPalette : public QDialog
{
    Q_OBJECT

public:
    explicit CommandPalette(QWidget *parent = nullptr);

    void addCommand(const Command &cmd);
    void clearCommands();
    void showPalette();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onSearchTextChanged(const QString &text);
    void executeSelected();

private:
    void setupUi();
    bool matchesSearch(const Command &cmd, const QString &search) const;

    // 应用主题
    void applyTheme();

    QLineEdit *m_searchEdit;
    QListWidget *m_listWidget;

    QVector<Command> m_commands;

    // 保存主题颜色（供列表项渲染用）
    QString m_shortcutColor;
    QString m_bgColor;
    QString m_fgColor;
};

#endif // COMMANDPALETTE_H