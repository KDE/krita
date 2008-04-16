var textEdit = new QTextEdit;
var document = textEdit.document();
var cursor = textEdit.textCursor();

var defaultFont = new QFont;
defaultFont.setPointSizeF(12.0); // See KoCharacterStyle.cpp

var defaultFormat = new QTextCharFormat;
defaultFormat.setFont(defaultFont);

var twentyEightPointFormat = new QTextCharFormat;
var twentyEightPointFont = new QFont(defaultFont);
twentyEightPointFont.setPointSizeF(28.0);
twentyEightPointFormat.setFont(twentyEightPointFont);

cursor.insertText("The following is a character whose font size is ", defaultFormat);
cursor.insertText("28.", twentyEightPointFormat);
cursor.insertText(" ", defaultFormat); // CHECKME: Why is a blank space appended?

cursor.insertBlock(); // CHECKME: This should not be created

return document;
