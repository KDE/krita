include("common.qs");

var font = defaultTextFormat.font();

var size28Format = QTextCharFormat.clone(defaultTextFormat);
font.setPointSizeF(28);
size28Format.setFont(font);

var size50pcFormat = QTextCharFormat.clone(defaultTextFormat);
font.setPointSizeF(defaultTextFormat.font().pointSizeF() * 50/100);
size50pcFormat.setFont(font);

cursor.insertText("This test case checks font size, this is an example of text with ", defaultTextFormat);
cursor.insertText("font size of 28", size28Format);
cursor.insertText(" ", defaultTextFormat);
cursor.insertBlock();
cursor.insertText("This test case checks font size, this is an example of text with ", defaultTextFormat);
cursor.insertText("font size of 50%", size50pcFormat);
cursor.insertText(" ", defaultTextFormat);

return document;
