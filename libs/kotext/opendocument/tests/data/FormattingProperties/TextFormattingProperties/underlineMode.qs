include("common.qs");

var font = defaultTextFormat.font();

var continuousUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(continuousUnderlineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.SolidLine);
setFormatProperty(continuousUnderlineFormat, KoCharacterStyle.UnderlineMode, KoCharacterStyle.ContinuousLineMode);
setFormatProperty(continuousUnderlineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

var wordUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(wordUnderlineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.SolidLine);
setFormatProperty(wordUnderlineFormat, KoCharacterStyle.UnderlineMode, KoCharacterStyle.SkipWhiteSpaceLineMode);
setFormatProperty(wordUnderlineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

cursor.insertText("This is an example of text with continuous underline.", continuousUnderlineFormat);
cursor.insertBlock();
cursor.insertText("This is an example of text with word only underline.", wordUnderlineFormat);

return document;
