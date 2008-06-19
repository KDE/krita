include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat, QTextFormat.BlockTopMargin, 72 * 0.2); // 1in = 72pt
setFormatProperty(textBlockFormat, QTextFormat.BlockBottomMargin, 72 * 0.2);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with top margin of 0.2 inch, and bottom margin of 0.2 inch. This is an example of paragraph with top margin of 0.2 inch, and bottom margin of 0.2 inch.");
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, QTextFormat.BlockTopMargin, 72 * 0.1);
setFormatProperty(textBlockFormat, QTextFormat.BlockBottomMargin, 72 * 0.2);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with top margin of 0.1 inch, and bottom margin of 0.2 inch. This is an example of paragraph with top margin of 0.1 inch, and bottom margin of 0.2 inch.");
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, QTextFormat.BlockTopMargin, 72 * 0.2);
setFormatProperty(textBlockFormat, QTextFormat.BlockBottomMargin, 72 * 0.1);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with top margin of 0.2 inch, and bottom margin of 0.1 inch. This is an example of paragraph with top margin of 0.2 inch, and bottom margin of 0.1 inch.");

return document;
