include("common.qs");

var font = defaultTextFormat.font();

var normalFormat = QTextCharFormat.clone(defaultTextFormat);
font.setStyle(QFont.StyleNormal);
normalFormat.setFont(font);

var italicFormat = QTextCharFormat.clone(defaultTextFormat);
font.setStyle(QFont.StyleItalic);
italicFormat.setFont(font);

var obliqueFormat = QTextCharFormat.clone(defaultTextFormat);
font.setStyle(QFont.StyleOblique);
obliqueFormat.setFont(font);

cursor.insertText("This is an example of text with normal font style. ", normalFormat);
cursor.insertBlock();
cursor.insertText("This is an example of text with italic font style. ", italicFormat);
cursor.insertBlock();
cursor.insertText("This is an example of text with oblique font style. ", obliqueFormat);

return document;
