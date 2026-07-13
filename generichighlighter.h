#ifndef GENERICHIGHLIGHTER_H
#define GENERICHIGHLIGHTER_H

#include "highlighter.h"
#include "syntaxdefinition.h"

class GenericHighlighter : public Highlighter
{
    Q_OBJECT

public:
    GenericHighlighter(const SyntaxDefinition &def, QTextDocument *parent = nullptr);
    void setupRules() override;
private:
    SyntaxDefinition m_definition; 
};

#endif // GENERICHIGHLIGHTER_H