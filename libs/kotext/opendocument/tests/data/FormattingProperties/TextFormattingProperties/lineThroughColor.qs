include("common.qs");

var blackStrikeThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(blackStrikeThroughFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.SolidLine);
setFormatProperty(blackStrikeThroughFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);
setFormatProperty(blackStrikeThroughFormat, KoCharacterStyle.StrikeOutColor, new QColor("#000000"));

var redStrikeThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(redStrikeThroughFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.SolidLine);
setFormatProperty(redStrikeThroughFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);
setFormatProperty(redStrikeThroughFormat, KoCharacterStyle.StrikeOutColor, new QColor("#ff3366"));

cursor.insertText("this is an example of text with black line through.", blackStrikeThroughFormat);
cursor.insertBlock();
cursor.insertText("this is an example of text with red line through.", redStrikeThroughFormat);

return document;
