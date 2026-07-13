#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include "theme.h"

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager& instance();
    const Theme& currentTheme() const { return m_currentTheme; }
    void setTheme(const Theme &theme);
    void setLightTheme();
    void setDarkTheme();
signals:
    void themeChanged();
private:
    ThemeManager();
    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;
    Theme m_currentTheme;
};

#endif // THEMEMANAGER_H