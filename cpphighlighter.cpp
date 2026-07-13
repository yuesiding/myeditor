#include "cpphighlighter.h"
#include "thememanager.h"

CppHighlighter::CppHighlighter(QTextDocument *parent):Highlighter(parent){
    setupRules();
}

void CppHighlighter::setupRules()
{
    const Theme &theme = ThemeManager::instance().currentTheme();//获取当前主题
    HighlightRule rule;
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(theme.keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);

    QStringList keywordPatterns = {
        "\\bchar\\b", "\\bclass\\b", "\\bconst\\b",
        "\\bdouble\\b", "\\benum\\b", "\\bexplicit\\b",
        "\\bfriend\\b", "\\binline\\b", "\\bint\\b",
        "\\blong\\b", "\\bnamespace\\b", "\\boperator\\b",
        "\\bprivate\\b", "\\bprotected\\b", "\\bpublic\\b",
        "\\bshort\\b", "\\bsignals\\b", "\\bsigned\\b",
        "\\bslots\\b", "\\bstatic\\b", "\\bstruct\\b",
        "\\btemplate\\b", "\\btypedef\\b", "\\btypename\\b",
        "\\bunion\\b", "\\bunsigned\\b", "\\bvirtual\\b",
        "\\bvoid\\b", "\\bvolatile\\b", "\\bbool\\b",
        "\\bif\\b", "\\belse\\b", "\\bfor\\b",
        "\\bwhile\\b", "\\bdo\\b", "\\bswitch\\b",
        "\\bcase\\b", "\\bdefault\\b", "\\bbreak\\b",
        "\\bcontinue\\b", "\\breturn\\b", "\\bnew\\b",
        "\\bdelete\\b", "\\bthis\\b", "\\btrue\\b",
        "\\bfalse\\b", "\\bnullptr\\b", "\\btry\\b",
        "\\bcatch\\b", "\\bthrow\\b", "\\busing\\b",
        "\\bauto\\b", "\\boverride\\b", "\\bfinal\\b"
    };

    for (const QString &pattern:keywordPatterns){
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        m_rules.append(rule);
    }
    //Qt类
    QTextCharFormat qtClassFormat;
    qtClassFormat.setForeground(theme.qtClassColor);
    qtClassFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = qtClassFormat;
    m_rules.append(rule);
    //预处理
    QTextCharFormat preprocessorFormat;
    preprocessorFormat.setForeground(theme.preprocessorColor);
    rule.pattern = QRegularExpression("^\\s*#[^\n]*");
    rule.format = preprocessorFormat;
    m_rules.append(rule);
    //数字
    QTextCharFormat numberFormat;
    numberFormat.setForeground(theme.numberColor);
    rule.pattern = QRegularExpression("\\b[0-9]+\\.?[0-9]*\\b");
    rule.format = numberFormat;
    m_rules.append(rule);
    //字符串
    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(theme.stringColor);
    rule.pattern = QRegularExpression("\".*?\"");
    rule.format = quotationFormat;
    m_rules.append(rule);
    //字符
    QTextCharFormat charFormat;
    charFormat.setForeground(theme.stringColor);
    rule.pattern = QRegularExpression("'.'");
    rule.format = charFormat;
    m_rules.append(rule);
    //函数
    QTextCharFormat functionFormat;
    functionFormat.setForeground(theme.functionColor);
    rule.pattern = QRegularExpression("\\b[A-Za-z_][A-Za-z0-9_]*(?=\\s*\\()");
    rule.format = functionFormat;
    m_rules.append(rule);
    //单行注释
    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(theme.commentColor);
    singleLineCommentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    m_rules.append(rule);
    //多行注释
    m_multiLineCommentFormat.setForeground(theme.commentColor);
    m_multiLineCommentFormat.setFontItalic(true);
    m_commentStartExpression = QRegularExpression("/\\*");
    m_commentEndExpression = QRegularExpression("\\*/");
}