include("common.qs");

var redFormat = defaultTextFormat;
redFormat.setForeground(new QBrush(new QColor("#ff3366")));
cursor.insertText("This is red", redFormat);

var greenFormat = defaultTextFormat;
greenFormat.setForeground(new QBrush(new QColor("#3deb3d")));
cursor.insertBlock();
cursor.insertText("This is green", greenFormat);

var blueFormat = defaultTextFormat;
blueFormat.setForeground(new QBrush(new QColor("#0066cc")));
cursor.insertBlock();
cursor.insertText("this is blue", blueFormat);

var blackFormat = defaultTextFormat;
blackFormat.setForeground(new QBrush(new QColor("#000000")));
cursor.insertBlock();
cursor.insertText("this is black", blackFormat);

cursor.insertBlock(new QTextBlockFormat, defaultTextFormat); // CHECKME: I don't think this should be created

return document;
