#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QString>
struct Theme {
    QString name;
    QColor editorBackground;
    QColor editorForeground;
    QColor lineNumberBackground;
    QColor lineNumberForeground;
    QColor currentLineHighlight;
    QColor bracketMatchColor;
    QColor keywordColor;   
    QColor qtClassColor;     
    QColor preprocessorColor; 
    QColor numberColor;       
    QColor stringColor;       
    QColor functionColor;     
    QColor commentColor;     
    QColor colorByName(const QString &name) const;
    static Theme lightTheme();
    static Theme darkTheme();
};

#endif // THEME_H