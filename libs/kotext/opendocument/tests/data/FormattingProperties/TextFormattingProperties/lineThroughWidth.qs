include("common.qs");

var tf = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(tf, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.DashLine);
setFormatProperty(tf, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

setFormatProperty(tf, KoCharacterStyle.StrikeOutWeight, KoCharacterStyle.AutoLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is auto.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutWeight, KoCharacterStyle.NormalLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is normal.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutWeight, KoCharacterStyle.BoldLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is bold.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutWeight, KoCharacterStyle.ThinLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is thin.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutWeight, KoCharacterStyle.DashLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is dash.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutWeight, KoCharacterStyle.MediumLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is medium.", tf);

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutWeight, KoCharacterStyle.ThickLineWeight);
cursor.insertText("This is an example of text with linethrough, the linethrough width is thick.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutWeight, KoCharacterStyle.PercentLineWeight);
setFormatProperty(tf, KoCharacterStyle.StrikeOutWidth, 100);
cursor.insertText("This is an example of text with linethrough, the linethrough width is 1.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutWeight, KoCharacterStyle.PercentLineWeight);
setFormatProperty(tf, KoCharacterStyle.StrikeOutWidth, 50);
cursor.insertText("This is an example of text with linethrough, the linethrough width is 50%.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutWeight, KoCharacterStyle.LengthLineWeight);
setFormatProperty(tf, KoCharacterStyle.StrikeOutWidth, 5);
cursor.insertText("This is an example of text with linethrough, the linethrough width is 5pt.", tf);

cursor.insertBlock();
cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutWeight, KoCharacterStyle.LengthLineWeight);
setFormatProperty(tf, KoCharacterStyle.StrikeOutWidth, 0.05 * 72);
cursor.insertText("This is an example of text with linethrough, the linethrough width is 0.05in.", tf);

return document;
