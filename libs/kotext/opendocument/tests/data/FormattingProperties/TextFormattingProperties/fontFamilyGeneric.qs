include("common.qs");

var romanFormat = QTextCharFormat.clone(defaultTextFormat);
var romanFont = romanFormat.font();
romanFont.setFamily("Roman");
romanFormat.setFont(romanFont);
setFormatProperty(romanFormat, 8163, 1); //for some strange reason neither format.setFontStyleHint(QFont.Serif) nor setFormatProperty(format, QTextFormat.FontStyleHint, QFont.Serif) work

var decorativeFormat = QTextCharFormat.clone(defaultTextFormat);
var decorativeFont = decorativeFormat.font();
decorativeFont.setFamily("decorative");
decorativeFormat.setFont(decorativeFont);
setFormatProperty(decorativeFormat, 8163, 3);

cursor.insertText("This is an example of using font family generic in text.", romanFormat);
cursor.insertBlock();
cursor.insertText("This is an example of using font family generic in text.", decorativeFormat);

document;
