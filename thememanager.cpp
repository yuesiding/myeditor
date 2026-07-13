#include "thememanager.h"

ThemeManager& ThemeManager::instance(){
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager(){
    m_currentTheme=Theme::lightTheme();
}

void ThemeManager::setTheme(const Theme &theme){
    m_currentTheme=theme;
    emit themeChanged(); 
}

void ThemeManager::setLightTheme(){
    setTheme(Theme::lightTheme());
}

void ThemeManager::setDarkTheme(){
    setTheme(Theme::darkTheme());
}