#ifndef CPPHIGHLIGHTER_H
#define CPPHIGHLIGHTER_H

#include "highlighter.h"

class CppHighlighter : public Highlighter
{
    Q_OBJECT

public:
    explicit CppHighlighter(QTextDocument *parent = nullptr);

    // 🆕 实现基类的虚函数
    void setupRules() override;
};

#endif // CPPHIGHLIGHTER_H