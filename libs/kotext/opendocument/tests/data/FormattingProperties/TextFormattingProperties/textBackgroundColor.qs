include("common.qs");

var transparentBackgroundFormat = QTextCharFormat.clone(defaultTextFormat);
transparentBackgroundFormat.setBackground(new QBrush(new QColor("#000000")));

var redBackgroundFormat = QTextCharFormat.clone(defaultTextFormat);
redBackgroundFormat.setBackground(new QBrush(new QColor("#ff3366")));

cursor.insertText("This is an example of text with transparent background color. ", transparentBackgroundFormat);
cursor.insertBlock();
cursor.insertText("This is an example of text with red background color. ", redBackgroundFormat);

return document;
