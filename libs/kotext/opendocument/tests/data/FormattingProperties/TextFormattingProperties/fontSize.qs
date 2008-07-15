include("common.qs");

var font = new QFont(defaultFont);
var size28Format = QTextCharFormat.clone(defaultTextFormat);
font.setPointSizeF(28);
size28Format.setFont(font);

font = new QFont(defaultFont);
var size50pcFormat = QTextCharFormat.clone(defaultTextFormat);
font.setPointSizeF(12.0 * 50/100); // ### hardcoded because KoCharacterStyle is hardcoded too..
size50pcFormat.setFont(font);

cursor.insertText("This test case checks font size, this is an example of text with ", defaultTextFormat);
cursor.insertText("font size of 28", size28Format);
cursor.insertBlock();
cursor.insertText("This test case checks font size, this is an example of text with ", defaultTextFormat);
cursor.insertText("font size of 50%", size50pcFormat);

return document;
