include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat, QTextFormat.BlockLeftMargin, 72 * 1); // 1in = 72pt
setFormatProperty(textBlockFormat, QTextFormat.BlockRightMargin, 72 * 1);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with left margin of 1 inch and right margin of 1 inch. This is an example of paragraph with left margin of 1 inch and right margin of 1 inch. ");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, QTextFormat.BlockLeftMargin, 72 * 1);
setFormatProperty(textBlockFormat, QTextFormat.BlockRightMargin, 72 * 2);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with left margin of 1 inch and right margin of 2 inch. This is an example of paragraph with left margin of 1 inch and right margin of 2 inch. ");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, QTextFormat.BlockLeftMargin, 72 * 2);
setFormatProperty(textBlockFormat, QTextFormat.BlockRightMargin, 72 * 1);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with left margin of 2 inch and right margin of 1 inch. This is an example of paragraph with left margin of 2 inch and right margin of 1 inch. ");

return document;
