include("common.qs");

var transparentBackgroundFormat = QTextBlockFormat.clone(defaultBlockFormat);
transparentBackgroundFormat.setBackground(new QBrush());

var redBackgroundFormat = QTextBlockFormat.clone(defaultBlockFormat);
redBackgroundFormat.setBackground(new QBrush(new QColor("#ff3366")));

var greenBackgroundFormat = QTextBlockFormat.clone(defaultBlockFormat);
greenBackgroundFormat.setBackground(new QBrush(new QColor("#3deb3d")));

cursor.setBlockFormat(transparentBackgroundFormat);
cursor.setCharFormat(QTextCharFormat.clone(defaultTextFormat));
cursor.insertText("This is an example of paragraph with transparent background color. ");
cursor.insertBlock(redBackgroundFormat);
cursor.insertText("This is an example of paragraph with red background color. ");;
cursor.insertBlock(greenBackgroundFormat);
cursor.insertText("This is an example of paragraph with green background color. ");

return document;
