include("common.qs");

var tf = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(tf, KoCharacterStyle.UnderlineStyle, KoCharacterStyle.DashLine);
setFormatProperty(tf, KoCharacterStyle.UnderlineType, KoCharacterStyle.SingleLine);

setFormatProperty(tf, KoCharacterStyle.UnderlineWeight, KoCharacterStyle.AutoLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is auto.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.UnderlineWeight, KoCharacterStyle.NormalLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is normal.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.UnderlineWeight, KoCharacterStyle.BoldLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is bold.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.UnderlineWeight, KoCharacterStyle.ThinLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is thin.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.UnderlineWeight, KoCharacterStyle.DashLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is dash.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.UnderlineWeight, KoCharacterStyle.MediumLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is medium.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.UnderlineWeight, KoCharacterStyle.ThickLineWeight);
cursor.insertText("This is an example of text with underline, the underline width is thick.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.UnderlineWeight, KoCharacterStyle.PercentLineWeight);
setFormatProperty(tf, KoCharacterStyle.UnderlineWidth, 100);
cursor.insertText("This is an example of text with underline, the underline width is 1.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.UnderlineWeight, KoCharacterStyle.PercentLineWeight);
setFormatProperty(tf, KoCharacterStyle.UnderlineWidth, 50);
cursor.insertText("This is an example of text with underline, the underline width is 50%.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.UnderlineWeight, KoCharacterStyle.LengthLineWeight);
setFormatProperty(tf, KoCharacterStyle.UnderlineWidth, 5);
cursor.insertText("This is an example of text with underline, the underline width is 5pt.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.UnderlineWeight, KoCharacterStyle.LengthLineWeight);
setFormatProperty(tf, KoCharacterStyle.UnderlineWidth, 0.05 * 72);
cursor.insertText("This is an example of text with underline, the underline width is 0.05in.", tf);

return document;
