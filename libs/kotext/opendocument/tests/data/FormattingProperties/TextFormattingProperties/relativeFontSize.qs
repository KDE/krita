include("common.qs");

var font = defaultTextFormat.font();

var sizeP5Format = QTextCharFormat.clone(defaultTextFormat); // size+5
font.setPointSizeF(12.0 + 5); // hardcoded since KoCharacterStyle is hardcoded too
sizeP5Format.setFont(font);

var sizeM3Format = QTextCharFormat.clone(defaultTextFormat); // size-3
font.setPointSizeF(12.0 - 3); // hardcoded since KoCharacterStyle is hardcoded too
sizeM3Format.setFont(font);

cursor.insertText("This test case checks font size, this is an example of text with ", defaultTextFormat);
cursor.insertText("relative font size of +5pt", sizeP5Format);
cursor.insertText("", defaultTextFormat);
cursor.insertBlock();
cursor.insertText("This test case checks font size, this is an example of text with ", defaultTextFormat);
cursor.insertText("relative font size of -3pt", sizeM3Format);
cursor.insertText("", defaultTextFormat);

return document;
