include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat, QTextFormat.BlockLeftMargin, 72 * 1); // 1in = 72pt
setFormatProperty(textBlockFormat, QTextFormat.BlockRightMargin, 72 * 1);
setFormatProperty(textBlockFormat, QTextFormat.BlockTopMargin, 72 * 1);
setFormatProperty(textBlockFormat, QTextFormat.BlockBottomMargin, 72 * 1);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with margin (top, bottom, left and right margins for paragraphs simultaneously) of 1 inch. This is an example of paragraph with margin (top, bottom, left and right margins for paragraphs simultaneously) of 1 inch.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, QTextFormat.BlockLeftMargin, 72 * 0.5);
setFormatProperty(textBlockFormat, QTextFormat.BlockRightMargin, 72 * 0.5);
setFormatProperty(textBlockFormat, QTextFormat.BlockTopMargin, 72 * 0.5);
setFormatProperty(textBlockFormat, QTextFormat.BlockBottomMargin, 72 * 0.5);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with margin (top, bottom, left and right margins for paragraphs simultaneously) of 0.5 inch. This is an example of paragraph with margin (top, bottom, left and right margins for paragraphs simultaneously) of 0.5 inch.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, QTextFormat.BlockLeftMargin, 72 * 0.1);
setFormatProperty(textBlockFormat, QTextFormat.BlockRightMargin, 72 * 0.1);
setFormatProperty(textBlockFormat, QTextFormat.BlockTopMargin, 72 * 0.1);
setFormatProperty(textBlockFormat, QTextFormat.BlockBottomMargin, 72 * 0.1);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with margin (top, bottom, left and right margins for paragraphs simultaneously) of 0.1 inch. This is an example of paragraph with margin (top, bottom, left and right margins for paragraphs simultaneously) of 0.1 inch.");

return document;
