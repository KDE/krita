include("common.qs");

var smallCapsFormat = QTextCharFormat.clone(defaultTextFormat);
smallCapsFormat.setFontCapitalization(QFont.SmallCaps);

var normalFormat = QTextCharFormat.clone(defaultTextFormat);
normalFormat.setFontCapitalization(QFont.MixedCase);

cursor.insertText("this is an example of paragraph displaying text as small capitalized letters on.", smallCapsFormat);
cursor.insertBlock();
cursor.insertText("this is an example of paragraph displaying text as small capitalized letters off.", normalFormat);

return document;
