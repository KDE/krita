include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
var t1in = new KoTextTab; t1in.position = 1 * 72; // 1 in = 72 pts
var t2in = new KoTextTab; t2in.position = 2 * 72;
var t3dot5in = new KoTextTab; t3dot5in.position = 3.5 * 72;
var t5in = new KoTextTab; t5in.position = 5 * 72;
var t100pt = new KoTextTab; t100pt.position = 100;

setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [t2in]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("some text\t(tab)\t(tab) position of tab stop is 2in. ");
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [t1in, t2in, t3dot5in, t5in]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("some text\t(tab)\t(tab) \t(tab) position of tab stops are 1in, 2in, 3.5in, 5in. ");
cursor.insertBlock(defaultBlockFormat);

setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [t100pt]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("some text\t(tab)\t(tab) position of tab stop is 100pt. ");

return document;
