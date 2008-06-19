include("common.qs");

var textOutlineFormat = QTextCharFormat.clone(defaultTextFormat);
textOutlineFormat.setForeground(new QBrush(new QColor(Qt.transparent)));
setFormatProperty(textOutlineFormat, QTextFormat.TextOutline, new QPen());

var noTextOutlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noTextOutlineFormat, QTextFormat.TextOutline, new QPen(Qt.NoPen));

cursor.insertText("this is an example of text displaying as an outline.", textOutlineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text displaying the text itself.", noTextOutlineFormat);

return document;
