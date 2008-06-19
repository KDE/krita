include("common.qs");

cursor.setCharFormat(defaultTextFormat);

var textBlockFormat = QTextBlockFormat.clone(defaultBlockFormat);
var textBlockFormatAi = new QTextBlockFormat;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabStopDistance, 72);

cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\ta paragraph whose (tab)\tdefault tab stop distance is 1in."); 

return document;
