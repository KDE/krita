include("common.qs");

var noLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noLineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.NoLineStyle);
setFormatProperty(noLineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

var solidLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(solidLineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.SolidLine);
setFormatProperty(solidLineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

var dottedLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dottedLineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.DottedLine);
setFormatProperty(dottedLineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

var dashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dashLineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.DashLine);
setFormatProperty(dashLineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

var longDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(longDashLineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.LongDashLine);
setFormatProperty(longDashLineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

var dotDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dotDashLineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.DashDotLine);
setFormatProperty(dotDashLineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

var dotDotDashLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(dotDotDashLineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.DashDotDotLine);
setFormatProperty(dotDotDashLineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

var waveLineFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(waveLineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.WaveLine);
setFormatProperty(waveLineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

cursor.insertText("This is an example of text without underline.", noLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with solid underline.", solidLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with dotted underline.", dottedLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with dash underline.", dashLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with long-dash underline.", longDashLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with dot dash underline.", dotDashLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with dot dot dash underline.", dotDotDashLineFormat);

cursor.insertBlock();
cursor.insertText("This is an example of text with wave underline.", waveLineFormat);

return document;
