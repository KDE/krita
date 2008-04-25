include("common.qs");

var font = defaultTextFormat.font();

var fontColorUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
fontColorUnderlineFormat.setForeground(new QBrush(new QColor("#FF3366")));
setFormatProperty(fontColorUnderlineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.SolidLine);
setFormatProperty(fontColorUnderlineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

var definedColorUnderlineFormat = QTextCharFormat.clone(defaultTextFormat);
definedColorUnderlineFormat.setForeground(new QBrush(new QColor("#FF3366")));
definedColorUnderlineFormat.setUnderlineColor(new QColor("#3deb3d"));
setFormatProperty(definedColorUnderlineFormat, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.SolidLine);
setFormatProperty(definedColorUnderlineFormat, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

cursor.insertText("This is an example of text with underline which has the same color of font. ", fontColorUnderlineFormat);
cursor.insertBlock();
cursor.insertText("This is an example of text with underline which has the defined color. ", definedColorUnderlineFormat);

return document;
