var textEdit = new QTextEdit;
var document = textEdit.document();
var cursor = textEdit.textCursor();

var defaultFont = new QFont;
defaultFont.setPointSizeF(12.0); // See KoCharacterStyle.cpp

var defaultFormat = new QTextCharFormat;
defaultFormat.setFont(defaultFont);

var redFormat = defaultFormat;
redFormat.setForeground(new QBrush(new QColor("#ff3366")));
cursor.insertText("this is an example of specifying the foreground color of the text.", redFormat);

cursor.insertText(" "); // CHECKME: Why is this appended?

cursor.insertBlock(new QTextBlockFormat, new QTextCharFormat); // CHECKME: I don't think this should be created

return document;
