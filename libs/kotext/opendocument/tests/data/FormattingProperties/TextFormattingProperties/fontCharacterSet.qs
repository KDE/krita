include("common.qs");

var symbolsFormat = QTextCharFormat.clone(defaultTextFormat);
var symbolsFont = symbolsFormat.font();
symbolsFont.setFamily("roman");
symbolsFormat.setFont(symbolsFont);
// i think the test is wrong. the value should be x-symbol and not 0xf001
setFormatProperty(symbolsFormat, KoCharacterStyle.FontCharset, "0xf001");

cursor.insertText("This test case checks text position, this is an example of text with ");
cursor.insertText("font character set.", symbolsFormat);
cursor.insertText(" ", defaultTextFormat);

return document;
