include("common.qs");

var noLineThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noLineThroughFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.SolidLine);
setFormatProperty(noLineThroughFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.NoLineType);
setFormatProperty(noLineThroughFormat, KoCharacterStyle.StrikeOutColor, new QColor("#000000"));

var singleLineThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(singleLineThroughFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.SolidLine);
setFormatProperty(singleLineThroughFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);
setFormatProperty(singleLineThroughFormat, KoCharacterStyle.StrikeOutColor, new QColor("#000000"));

var doubleLineThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(doubleLineThroughFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.SolidLine);
setFormatProperty(doubleLineThroughFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.DoubleLine);
setFormatProperty(doubleLineThroughFormat, KoCharacterStyle.StrikeOutColor, new QColor("#000000"));

cursor.insertText("this is an example of text with no line trough ", noLineThroughFormat); // typo intentional

cursor.insertBlock();
cursor.insertText("this is an example of text with single line trough ", singleLineThroughFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with double line trough ", doubleLineThroughFormat);

return document;

