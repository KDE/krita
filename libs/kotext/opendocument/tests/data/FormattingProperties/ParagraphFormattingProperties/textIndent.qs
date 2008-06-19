include("common.qs");

var font = defaultTextFormat.font();
// font.setPointSize("12");

var textCharFormat = QTextCharFormat.clone(defaultTextFormat);
textCharFormat.setFont(font);
cursor.setCharFormat(textCharFormat);

var textBlockFormat = QTextBlockFormat.clone(defaultBlockFormat);
var indent = (new QFontMetrics(font)).width('x') * 3; // 3 is just what used in code
textCharFormat.setFont(font);
cursor.setCharFormat(textCharFormat);

textBlockFormat.setTextIndent(72 * 1); // 1 inch = 72pt
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("This is an example of paragraph with text indent of 1 inch. This is an example of paragraph with text indent of 1 inch.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

textBlockFormat.setTextIndent(72 * -1);
cursor.insertBlock(textBlockFormat);
cursor.insertText("This is an example of paragraph with text indent of -1 inch. This is an example of paragraph with text indent of -1 inch.");
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

textBlockFormat.setTextIndent(72 * 2);
cursor.insertBlock(textBlockFormat);
cursor.insertText("This is an example of paragraph with text indent of 2 inch. This is an example of paragraph with text indent of 2 inch.");

return document;
