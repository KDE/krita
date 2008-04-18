// this is very similar to boldAndItalic.qs

include("common.qs");

var italicFormat = QTextCharFormat.clone(defaultTextFormat);
italicFormat.setFont(defaultFont);
italicFormat.setFontItalic(true);

var boldFormat = QTextCharFormat.clone(defaultTextFormat);
var boldFont = new QFont(defaultFont);
boldFont.setBold(true);
boldFormat.setFont(boldFont);

cursor.insertText("In this paragraphs ", defaultTextFormat);
cursor.insertText("these words are emphasized", italicFormat);
cursor.insertText(" and ", defaultTextFormat);
cursor.insertText("these words are emphasized", boldFormat);
cursor.insertText(". ", defaultTextFormat);

return document;
