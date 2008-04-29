include("common.qs");

var romanFormat = QTextCharFormat.clone(defaultTextFormat);
var romanFont = romanFormat.font();
romanFont.setFamily("Roman");
romanFont.setStyleHint(QFont.Serif);
romanFormat.setFont(romanFont);

var decorativeFormat = QTextCharFormat.clone(defaultTextFormat);
var decorativeFont = decorativeFormat.font();
decorativeFont.setFamily("decorative");
decorativeFont.setStyleHint(QFont.Decorative);
decorativeFormat.setFont(decorativeFont);

cursor.insertText("This is an example of using font family generic in text. ", romanFormat);
cursor.insertBlock();
cursor.insertText("This is an example of using font family generic in text. ", decorativeFormat);

return document;
