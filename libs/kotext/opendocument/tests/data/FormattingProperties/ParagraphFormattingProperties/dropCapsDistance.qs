include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCaps, true);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsLines, 2);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsLength, 1);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsDistance, 0);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 0in. This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 0in.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsDistance, 72 * 1); // 1in=72pt
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 1in. This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 1in.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsDistance, 72 * 3);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 3in. This is an example of paragraph with drop caps. the distance between the last dropped character and the first of the remaining characters of each line is 3in.");

return document;
