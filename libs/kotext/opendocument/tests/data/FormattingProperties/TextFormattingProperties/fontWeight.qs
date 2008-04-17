include("common.qs");

var font = defaultTextFormat.font();

var normalFormat = QTextCharFormat.clone(defaultTextFormat);
font.setWeight(QFont.Normal);
normalFormat.setFont(font);

var boldFormat = QTextCharFormat.clone(defaultTextFormat);
font.setWeight(QFont.Bold);
boldFormat.setFont(font);

var weight100Format = QTextCharFormat.clone(defaultTextFormat);
font.setWeight(10);
weight100Format.setFont(font);

var weight500Format = QTextCharFormat.clone(defaultTextFormat);
font.setWeight(50);
weight500Format.setFont(font);

var weight900Format = QTextCharFormat.clone(defaultTextFormat);
font.setWeight(90);
weight900Format.setFont(font);

cursor.insertText("This is an example of text with normal font weight", normalFormat);
cursor.insertBlock();
cursor.insertText("This is an example of text with bold font weight", boldFormat);
cursor.insertBlock();
cursor.insertText("This is an example of text with 100 font weight", weight100Format);
cursor.insertBlock();
cursor.insertText("This is an example of text with 500 font weight", weight500Format);
cursor.insertBlock();
cursor.insertText("This is an example of text with 900 font weight", weight900Format);

cursor.insertBlock(new QTextBlockFormat, new QTextCharFormat); // CHECKME: I don't think this should be created

return document;
