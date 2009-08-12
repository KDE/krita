include("common.qs");

var noLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noLineFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.NoLineStyle);

var solidLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(solidLineFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.SolidLine);
setFormatProperty(solidLineFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

var dottedLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dottedLineFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.DottedLine);
setFormatProperty(dottedLineFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

var dashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dashLineFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.DashLine);
setFormatProperty(dashLineFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

var longDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(longDashLineFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.LongDashLine);
setFormatProperty(longDashLineFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

var dotDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dotDashLineFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.DashDotLine);
setFormatProperty(dotDashLineFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

var dotDotDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dotDotDashLineFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.DashDotDotLine);
setFormatProperty(dotDotDashLineFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

var waveLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(waveLineFormat, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.WaveLine);
setFormatProperty(waveLineFormat, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

cursor.insertText("this is an example of text without line through.", noLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is solid.", solidLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is dotted.", dottedLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is dash.", dashLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is long dash.", longDashLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is dot dash.", dotDashLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is dot dot dash.", dotDotDashLineFormat);

cursor.insertBlock();
cursor.insertText("this is an example of text with line trough whose style is wave.", waveLineFormat);

document;
