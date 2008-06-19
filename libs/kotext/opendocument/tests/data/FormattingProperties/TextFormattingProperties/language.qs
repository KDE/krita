include("common.qs");

var chineseFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(chineseFormat, KoCharacterStyle.Language, "zh");

cursor.insertText("This is an example of text with different language. the following is Chinese word which means \"how are you\".");
cursor.insertBlock();
cursor.insertText("你好", chineseFormat);

return document;
