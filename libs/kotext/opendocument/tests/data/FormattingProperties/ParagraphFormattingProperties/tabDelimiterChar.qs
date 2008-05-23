include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
var tabstop = new KoTextTab;

tabstop.position = 3 * 72; // 1 in = 72 pts
tabstop.type = QTextOption.DelimiterTab;
tabstop.delimiter = ",";
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("\tGamgee, Sam ");
cursor.insertBlock(textBlockFormat);
cursor.insertText("\tBrandybuck, Meriadoc ");
cursor.insertBlock(textBlockFormat);
cursor.insertText("\tTook, Peregrin ");

cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

tabstop.position = 2 * 72; // 1 in = 72 pts
tabstop.type = QTextOption.DelimiterTab;
tabstop.delimiter = "=";
var tabstop2 = new KoTextTab;
tabstop2.position = 3 * 72; // 1 in = 72 pts
tabstop2.type = QTextOption.DelimiterTab;
tabstop2.delimiter = ".";
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop, tabstop2]);
cursor.insertBlock(textBlockFormat);
cursor.insertText("\t0.322 + 4 = \t4.322 ");
cursor.insertBlock(textBlockFormat);
cursor.insertText("\t42 + 0.01 = \t42.01 ");
cursor.insertBlock(textBlockFormat);
cursor.insertText("\t4233.343 + 0.00 = \t4233.343 ");

return document;
