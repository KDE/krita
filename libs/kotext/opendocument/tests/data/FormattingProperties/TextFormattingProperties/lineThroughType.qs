include("common.qs");

var noLineThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noLineThroughFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.NoLineStyle);
setFormatProperty(noLineThroughFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.NoLineTYpe);

var singleLineThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(singleLineThroughFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.SolidLine);
setFormatProperty(singleLineThroughFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

var doubleLineThroughFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(doubleLineThroughFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.SolidLine);
setFormatProperty(doubleLineThroughFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.DoubleLine);

cursor.insertText("this is an example of text with no line trough", noLineThroughFormat); // typo intentional

cursor.insertBlock();
cursor.insertText("this is an example of text with single line trough", singleLineThroughFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with double line trough", doubleLineThroughFormat);

document;

