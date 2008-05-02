include("common.qs");

var chineseFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(chineseFormat, KoCharacterStyle.Country, "CN");

cursor.insertText("This is an example of specify the country of the text, by using style:country-asian=\"CN\" and style:language-asian=\"zh\" togather. ");
cursor.insertBlock();
cursor.insertText("你好 ", chineseFormat);

return document;
