// this is very similar to boldAndItalic.qs

var textEdit = new QTextEdit;
var document = textEdit.document();
var cursor = textEdit.textCursor();

var defaultFont = new QFont;
defaultFont.setPointSizeF(12.0); // See KoCharacterStyle.cpp

var defaultFormat = new QTextCharFormat;
defaultFormat.setFont(defaultFont);

var italicFormat = new QTextCharFormat;
italicFormat.setFont(defaultFont);
italicFormat.setFontItalic(true);

var boldFormat = new QTextCharFormat;
var boldFont = new QFont(defaultFont);
boldFont.setBold(true);
boldFormat.setFont(boldFont);

cursor.insertText("In this paragraphs ", defaultFormat);
cursor.insertText("these words are emphasized", italicFormat);
cursor.insertText(" and ", defaultFormat);
cursor.insertText("these words are emphasized", boldFormat);
cursor.insertText(". ", defaultFormat);

cursor.insertBlock(); // CHECKME: This should not be created

return document;
