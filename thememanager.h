#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include "theme.h"

// 主题管理器（单例）
// 用法：ThemeManager::instance().currentTheme()
class ThemeManager : public QObject
{
    Q_OBJECT

public:
    // 获取全局唯一实例
    static ThemeManager& instance();

    // 当前主题
    const Theme& currentTheme() const { return m_currentTheme; }

    // 切换主题
    void setTheme(const Theme &theme);

    // 便捷方法：切换到亮/暗
    void setLightTheme();
    void setDarkTheme();

signals:
    // 主题变化时发出信号，让所有编辑器刷新
    void themeChanged();

private:
    // 私有构造函数（单例）
    ThemeManager();
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    Theme m_currentTheme;
};

#endif // THEMEMANAGER_H