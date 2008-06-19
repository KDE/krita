include("common.qs");

var textBlockFormatBall = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormatBall, KoParagraphStyle.LeftBorderWidth, 0.0346 * 72);
setFormatProperty(textBlockFormatBall, KoParagraphStyle.TopBorderWidth, 0.0346 * 72);
setFormatProperty(textBlockFormatBall, KoParagraphStyle.RightBorderWidth, 0.0346 * 72);
setFormatProperty(textBlockFormatBall, KoParagraphStyle.BottomBorderWidth, 0.0346 * 72);
cursor.setBlockFormat(textBlockFormatBall);
cursor.insertText("This is an example of paragraph with border. The line arrangement is top, bottom, left and right. This is an example of paragraph with border. The line arrangement is top, bottom, left and right. This is an example of paragraph with border. The line arrangement is top, bottom, left and right.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

var textBlockFormatBtb = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormatBtb, KoParagraphStyle.TopBorderWidth, 0.0346 * 72);
setFormatProperty(textBlockFormatBtb, KoParagraphStyle.BottomBorderWidth, 0.0346 * 72);
cursor.setBlockFormat(textBlockFormatBtb);
cursor.insertText("This is an example of paragraph with border. The line arrangement is top and bottom. This is an example of paragraph with border. The line arrangement is top and bottom. This is an example of paragraph with border. The line arrangement is top and bottom.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

var textBlockFormatBlr = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormatBlr, KoParagraphStyle.LeftBorderWidth, 0.0346 * 72);
setFormatProperty(textBlockFormatBlr, KoParagraphStyle.RightBorderWidth, 0.0346 * 72);
cursor.setBlockFormat(textBlockFormatBlr);
cursor.insertText("This is an example of paragraph with border. The line arrangement is left and right. This is an example of paragraph with border. The line arrangement is left and right. This is an example of paragraph with border. The line arrangement is left and right.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

var textBlockFormatBr = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormatBr, KoParagraphStyle.RightBorderWidth, 0.0346 * 72);
cursor.setBlockFormat(textBlockFormatBr);
cursor.insertText("This is an example of paragraph with border. The line arrangement is right only. This is an example of paragraph with border. The line arrangement is right only. This is an example of paragraph with border. The line arrangement is right only.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

var textBlockFormatBb = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormatBb, KoParagraphStyle.BottomBorderWidth, 0.0346 * 72);
cursor.setBlockFormat(textBlockFormatBb);
cursor.insertText("This is an example of paragraph with border. The line arrangement is bottom only. This is an example of paragraph with border. The line arrangement is bottom only. This is an example of paragraph with border. The line arrangement is bottom only.");

return document;
