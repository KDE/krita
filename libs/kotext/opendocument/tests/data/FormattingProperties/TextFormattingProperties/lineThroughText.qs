include("common.qs");

var tf_plain = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(tf_plain, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.NoLineStyle);
setFormatProperty(tf_plain, KoCharacterStyle.StrikeOutText, "/");

var tf = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(tf, KoCharacterStyle.StrikeOutStyle, KoCharacterStyle.SolidLine);
setFormatProperty(tf, KoCharacterStyle.StrikeOutType, KoCharacterStyle.SingleLine);

setFormatProperty(tf, KoCharacterStyle.StrikeOutText, "/");
cursor.insertText("this is an example of text with line through /", tf);
cursor.insertBlock();
cursor.insertText("the line through text is /.", tf_plain);
cursor.insertBlock();

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutText, "x");
cursor.insertText("this is an example of text with line through x", tf);
cursor.insertBlock();
cursor.insertText("the line through text is x.", tf_plain);
cursor.insertBlock();

cursor.insertBlock();
setFormatProperty(tf, KoCharacterStyle.StrikeOutText, "--x--");
cursor.insertText("this is an example of text with line through --x--", tf);
cursor.insertBlock();
cursor.insertText("the line through text is --x--.", tf_plain);

document;
