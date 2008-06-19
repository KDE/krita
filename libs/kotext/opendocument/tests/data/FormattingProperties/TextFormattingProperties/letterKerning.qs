include("common.qs");

var kerningFormat = QTextCharFormat.clone(defaultTextFormat);
var kerningFont = kerningFormat.font();
kerningFont.setKerning(true);
kerningFormat.setFont(kerningFont);

var nonKerningFormat = QTextCharFormat.clone(defaultTextFormat);
var nonKerningFont = nonKerningFormat.font();
nonKerningFont.setKerning(false);
nonKerningFormat.setFont(nonKerningFont);

cursor.insertText("This is an example of text without letter kerning: Tr Ke Yr r, To AY y, AT", nonKerningFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertText("This is an example of text with letter kerning: Tr Ke Yr r, To AY y, AT", kerningFormat);

return document;
