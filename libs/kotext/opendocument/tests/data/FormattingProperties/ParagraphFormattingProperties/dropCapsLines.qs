include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCaps, true);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsLines, 1);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsLength, 1);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsDistance, 0);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 1. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 1. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 1. ");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsLines, 2);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 2. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 2. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 2. ");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsLines, 3);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 3. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 3. This is an example of paragraph with drop caps. The number of lines that the dropped characters should encircle is 3. ");

return document;
