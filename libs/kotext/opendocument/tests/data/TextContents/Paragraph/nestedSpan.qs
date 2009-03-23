// this is very similar to boldAndItalic.qs

include("common.qs");

var italicsmallFormat = QTextCharFormat.clone(defaultTextFormat);
italicsmallFormat.setFontPointSize(10);
italicsmallFormat.setFontItalic(true);

var italicDefaultFormat = QTextCharFormat.clone(defaultTextFormat);
italicDefaultFormat.setFontItalic(true);

var smallFormat = QTextCharFormat.clone(defaultTextFormat);
smallFormat.setFontPointSize(10);

cursor.insertText("In this paragraph ", defaultTextFormat);
cursor.insertText("these words are small", smallFormat);
cursor.insertText(" and ", defaultTextFormat);
cursor.insertText("these words are italic", italicDefaultFormat);
cursor.insertText(".", defaultTextFormat);
cursor.insertBlock(defaultTextFormat);
cursor.insertText("In this paragraph ", defaultTextFormat);
cursor.insertText("these words are small and ", smallFormat);
cursor.insertText("these words are small and italic", italicsmallFormat);
cursor.insertText(".", defaultTextFormat);

document;
