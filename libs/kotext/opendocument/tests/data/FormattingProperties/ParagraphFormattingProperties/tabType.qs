include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
var tabstop = new KoTextTab;
tabstop.position = 3 * 72; // 1 in = 72 pts

tabstop.type = QTextOption.LeftTab;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\ttab type is left.");
cursor.insertBlock(defaultBlockFormat);

tabstop.type = QTextOption.CenterTab;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\ttab type is center.");
cursor.insertBlock(defaultBlockFormat);

tabstop.type = QTextOption.RightTab;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\ttab type is right.");

return document;
