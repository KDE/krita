var textEdit = new QTextEdit;
var document = textEdit.document();
var cursor = textEdit.textCursor();

var defaultFont = new QFont;
defaultFont.setPointSizeF(12.0); // See KoCharacterStyle.cpp

var defaultFormat = new QTextCharFormat;
defaultFormat.setFont(defaultFont);

var redFormat = defaultFormat;
redFormat.setForeground(new QBrush(new QColor("#ff3366")));
cursor.insertText("This is red", redFormat);

var greenFormat = defaultFormat;
greenFormat.setForeground(new QBrush(new QColor("#3deb3d")));
cursor.insertBlock();
cursor.insertText("This is green", greenFormat);

var blueFormat = defaultFormat;
blueFormat.setForeground(new QBrush(new QColor("#0066cc")));
cursor.insertBlock();
cursor.insertText("this is blue", blueFormat);

var blackFormat = defaultFormat;
blackFormat.setForeground(new QBrush(new QColor("#000000")));
cursor.insertBlock();
cursor.insertText("this is black", blackFormat);

cursor.insertBlock(new QTextBlockFormat, new QTextCharFormat); // CHECKME: I don't think this should be created

return document;
