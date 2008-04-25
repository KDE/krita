include("common.qs");

var noUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noUnderlineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.SolidLine);
setFormatProperty(noUnderlineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.NoLineType);

var singleUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(singleUnderlineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.SolidLine);
setFormatProperty(singleUnderlineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

var doubleUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(doubleUnderlineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.SolidLine);
setFormatProperty(doubleUnderlineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.DoubleLine);

cursor.insertText("This is an example of text ", defaultTextFormat);
cursor.insertText("without underline.", noUnderlineFormat);
cursor.insertText(" ", defaultTextFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text ", defaultTextFormat);
cursor.insertText("with single underline.", singleUnderlineFormat);
cursor.insertText(" ", defaultTextFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text ", defaultTextFormat);
cursor.insertText("with double underline.", doubleUnderlineFormat);
cursor.insertText(" ", defaultTextFormat);

return document;
