include("common.qs");

var font = defaultTextFormat.font();

var continuousStrikeOutFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(continuousStrikeOutFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.SolidLine);
setFormatProperty(continuousStrikeOutFormat, KoCharacterStyle.StrikeOutMode, KoCharacterStyle.ContinuousLineMode);
setFormatProperty(continuousStrikeOutFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

var wordStrikeOutFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(wordStrikeOutFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.SolidLine);
setFormatProperty(wordStrikeOutFormat, KoCharacterStyle.StrikeOutMode, KoCharacterStyle.SkipWhiteSpaceLineMode);
setFormatProperty(wordStrikeOutFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

cursor.insertText("This is an example of text with continuous line through. ", continuousStrikeOutFormat);
cursor.insertBlock();
cursor.insertText("This is an example of text with word only line through. ", wordStrikeOutFormat);

return document;
