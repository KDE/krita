include("common.qs");

var font = defaultTextFormat.font();

var sizeP5Format = QTextCharFormat.clone(defaultTextFormat); // size+5
font.setPointSizeF(font.pointSizeF() + 5);
sizeP5Format.setFont(font);

var sizeM3Format = QTextCharFormat.clone(defaultTextFormat); // size-3
font.setPointSizeF(defaultTextFormat.font().pointSizeF() - 3);
sizeM3Format.setFont(font);

cursor.insertText("This test case checks font size, this is an example of text with ", defaultTextFormat);
cursor.insertText("relative font size of +5pt", sizeP5Format);
cursor.insertText(" ", defaultTextFormat);
cursor.insertBlock();
cursor.insertText("This test case checks font size, this is an example of text with ", defaultTextFormat);
cursor.insertText("relative font size of -3pt", sizeM3Format);
cursor.insertText(" ", defaultTextFormat);

return document;
