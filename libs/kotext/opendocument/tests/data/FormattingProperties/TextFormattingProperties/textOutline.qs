include("common.qs");

var textOutlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(textOutlineFormat, QTextFormat.TextOutline, new QPen(Qt.black, 0));
textOutlineFormat.setForeground(new QBrush(new QColor(Qt.transparent)));

var noTextOutlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noTextOutlineFormat, QTextFormat.TextOutline, new QPen(Qt.NoPen));

cursor.insertText("this is an example of text displaying as an outline.", textOutlineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text displaying the text itself.", noTextOutlineFormat);

return document;
