var textEdit = new QTextEdit;
var document = textEdit.document();
var cursor = textEdit.textCursor();

var defaultFont = new QFont;
defaultFont.setPointSizeF(12.0); // See KoCharacterStyle.cpp

var defaultFormat = new QTextCharFormat;
defaultFormat.setFont(defaultFont);

var emphasizedFormat = new QTextCharFormat;
var emphasizedFont = new QFont(defaultFont);
emphasizedFont.setBold(true);
emphasizedFormat.setFont(emphasizedFont);

var italicFormat = new QTextCharFormat;
italicFormat.setFont(defaultFont);
italicFormat.setFontItalic(true);

cursor.insertText("This word is ", defaultFormat);
cursor.insertText("emphasized", emphasizedFormat);
cursor.insertText(", and this word is ", defaultFormat);
cursor.insertText("italic", italicFormat);
cursor.insertText(". ", defaultFormat);

cursor.insertBlock(); // CHECKME: This should not be created

return document;
