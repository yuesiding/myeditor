#ifndef CPPHIGHLIGHTER_H
#define CPPHIGHLIGHTER_H

#include "highlighter.h"

class CppHighlighter:public Highlighter
{
    Q_OBJECT
public:
    explicit CppHighlighter(QTextDocument *parent = nullptr);
    void setupRules() override;
};

#endif // CPPHIGHLIGHTER_H