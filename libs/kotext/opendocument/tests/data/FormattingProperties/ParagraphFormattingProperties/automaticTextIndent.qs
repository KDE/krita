include("common.qs");

var font = defaultTextFormat.font();
font.setPointSize("20");

var textCharFormat = QTextCharFormat.clone(defaultTextFormat);
textCharFormat.setFont(font);
cursor.setCharFormat(textCharFormat);

var textBlockFormat = new QTextBlockFormat;
textBlockFormat.setTextIndent(48);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with font size 20pt, text indent of 1inch (ineffective) and automatic text indent enabled. This should just look, er, big. [P1_20_1in_auto] ");
cursor.insertBlock(new QTextBlockFormat);
cursor.insertBlock(new QTextBlockFormat);

cursor.setCharFormat(textCharFormat);
textBlockFormat.setTextIndent(48);
cursor.insertBlock(textBlockFormat);
cursor.insertText("This is an example of paragraph with font size 20pt, text indent of 4inch (ineffective) and automatic text indent enabled. This should visually have the same indentation as P1_20_1in_auto. [P2_20_4in_auto] ");
cursor.insertBlock(new QTextBlockFormat);
cursor.insertBlock(new QTextBlockFormat);

font.setPointSize("12");
textCharFormat.setFont(font);

cursor.setCharFormat(textCharFormat);
textBlockFormat.setTextIndent(30);
cursor.insertBlock(textBlockFormat);
cursor.insertText("This is an example of paragraph with font size 12pt, text indent of 1inch (ineffective) and automatic text indent enabled. This should visually have lesser indentation (proportional to the font size decrease) than P1_20_1in_auto. [P3_12_1in_auto] ");
cursor.insertBlock(new QTextBlockFormat);
cursor.insertBlock(new QTextBlockFormat);

textBlockFormat.setTextIndent(72);
cursor.insertBlock(textBlockFormat);
cursor.insertText("This is an example of paragraph with font size 12pt, text indent of 1inch and automatic text indent disabled. This should visually have 1 inch indentation. [P4_12_1in] ");
cursor.insertBlock(new QTextBlockFormat);
cursor.insertBlock(new QTextBlockFormat);

textBlockFormat.setTextIndent(72);
cursor.insertBlock(textBlockFormat);
cursor.insertText("This is an example of paragraph with font size 12pt, text indent of 1inch and automatic text indent unspecified. This should visually have 1 inch indentation, same as P4_12_1in. [P5_12_1in] ");

return document;
