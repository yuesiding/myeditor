#ifndef GENERICHIGHLIGHTER_H
#define GENERICHIGHLIGHTER_H

#include "highlighter.h"
#include "syntaxdefinition.h"

// 通用高亮器：根据 SyntaxDefinition 动态生成规则
class GenericHighlighter : public Highlighter
{
    Q_OBJECT

public:
    // 传入语法定义
    GenericHighlighter(const SyntaxDefinition &def, QTextDocument *parent = nullptr);

    // 实现基类的虚函数
    void setupRules() override;

private:
    SyntaxDefinition m_definition;  // 保存这份语法定义
};

#endif // GENERICHIGHLIGHTER_H