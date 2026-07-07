#include "thememanager.h"

// ===== 单例实例 =====
ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

// ===== 私有构造函数：默认亮主题 =====
ThemeManager::ThemeManager()
{
    m_currentTheme = Theme::lightTheme();
}

// ===== 切换主题 =====
void ThemeManager::setTheme(const Theme &theme)
{
    m_currentTheme = theme;
    emit themeChanged();  // 通知所有订阅者
}

void ThemeManager::setLightTheme()
{
    setTheme(Theme::lightTheme());
}

void ThemeManager::setDarkTheme()
{
    setTheme(Theme::darkTheme());
}