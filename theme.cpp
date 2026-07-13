#include "theme.h"

//亮
Theme Theme::lightTheme()
{
    Theme t;
    t.name = "Light";
    t.editorBackground = QColor("#FFFFFF");  
    t.editorForeground = QColor("#000000");    
    t.lineNumberBackground = QColor("#F0F0F0");
    t.lineNumberForeground = QColor("#808080");
    t.currentLineHighlight = QColor("#FFFACD"); 
    t.bracketMatchColor = QColor("#B3D9FF");    
    //highlight
    t.keywordColor = QColor("#0000FF");       
    t.qtClassColor = QColor("#008080");        
    t.preprocessorColor = QColor("#800080");  
    t.numberColor = QColor("#098658");          
    t.stringColor = QColor("#A31515");          
    t.functionColor = QColor("#795E26");        
    t.commentColor = QColor("#008000");       
    return t;
}

//暗
Theme Theme::darkTheme()
{
    Theme t;
    t.name = "Dark";
    t.editorBackground = QColor("#1E1E1E");    
    t.editorForeground = QColor("#D4D4D4");     
    t.lineNumberBackground = QColor("#252526");
    t.lineNumberForeground = QColor("#858585");
    t.currentLineHighlight = QColor("#2A2A2A"); 
    t.bracketMatchColor = QColor("#264F78");   
    //hightlight
    t.keywordColor = QColor("#569CD6");         
    t.qtClassColor = QColor("#4EC9B0");        
    t.preprocessorColor = QColor("#C586C0");    
    t.numberColor = QColor("#B5CEA8");         
    t.stringColor = QColor("#CE9178");         
    t.functionColor = QColor("#DCDCAA");        
    t.commentColor = QColor("#6A9955");       
    return t;
}
QColor Theme::colorByName(const QString &name) const
{
    if (name == "keyword") return keywordColor;
    if (name == "qtClass") return qtClassColor;
    if (name == "preprocessor") return preprocessorColor;
    if (name == "number") return numberColor;
    if (name == "string") return stringColor;
    if (name == "function") return functionColor;
    if (name == "comment") return commentColor;
    if (name == "foreground") return editorForeground;
    return editorForeground;
}