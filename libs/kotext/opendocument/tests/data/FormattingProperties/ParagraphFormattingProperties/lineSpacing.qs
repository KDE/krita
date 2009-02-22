include("common.qs");

var textBlockFormat1 = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat1, KoParagraphStyle.FixedLineHeight, 72 * 0.5);
cursor.setBlockFormat(textBlockFormat1);
cursor.insertText("This is a text with line-height of 0.5in.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

var textBlockFormat2 = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat2, KoParagraphStyle.PercentLineHeight, 200);
cursor.setBlockFormat(textBlockFormat2);
cursor.insertText("This is a text with line-height of 200%.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

var textBlockFormat3 = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat3, KoParagraphStyle.MinimumLineHeight, 72 * 0.5);
cursor.setBlockFormat(textBlockFormat3);
cursor.insertText("This is a text with minimal line-height of 0.5in.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

var textBlockFormat4 = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat4, KoParagraphStyle.LineSpacing, 72 * 0.5);
cursor.setBlockFormat(textBlockFormat4);
cursor.insertText("This is a text with line-spacing of 0.5in.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

var textBlockFormat5 = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat5, KoParagraphStyle.LineSpacingFromFont, true);
cursor.setBlockFormat(textBlockFormat5);
cursor.insertText("This is a text with font independent line height calculation.");

document;
