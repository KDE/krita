include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCaps, true);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsLines, 2);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsLength, 1);
setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsDistance, 0);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. This is also an example of drop caps with length of 1. This is an example of paragraph with drop caps. This is also an example of drop caps with length of 1. This is an example of paragraph with drop caps. This is also an example of drop caps with length of 1.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsLength, 2);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. This is also an example of drop caps with length of 2. This is an example of paragraph with drop caps. This is also an example of drop caps with length of 2. This is an example of paragraph with drop caps. This is also an example of drop caps with length of 2.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KoParagraphStyle.DropCapsLength, -1);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with drop caps. This is also an example of drop caps with length of word. This is an example of paragraph with drop caps. This is also an example of drop caps with length of word. This is an example of paragraph with drop caps. This is also an example of drop caps with length of word.");

return document;
