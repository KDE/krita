// This is very similar to attributedText.qs

include("common.qs");

var emphasizedFormat = QTextCharFormat.clone(defaultTextFormat);
var emphasizedFont = new QFont(defaultFont);
emphasizedFont.setBold(true);
emphasizedFormat.setFont(emphasizedFont);

var italicFormat = QTextCharFormat.clone(defaultTextFormat);
italicFormat.setFont(defaultFont);
italicFormat.setFontItalic(true);

cursor.insertText("This word is ", defaultTextFormat);
cursor.insertText("emphasized", emphasizedFormat);
cursor.insertText(", and this word is ", defaultTextFormat);
cursor.insertText("italic", italicFormat);
cursor.insertText(". ", defaultTextFormat);

cursor.insertBlock(); // CHECKME: This should not be created

return document;
