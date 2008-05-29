include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
var tabstop = new KoTextTab;
tabstop.position = 3 * 72; // 1 in = 72 pts
tabstop.leaderStyle = KoCharacterStyle.SolidLine;

tabstop.leaderText = "$";
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of paragraph with tab stop at 3in, whose leader line text is \"$\". ");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderText = "@";
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of paragraph with tab stop at 3in, whose leader line text is \"@\". ");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderText = "x";
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of paragraph with tab stop at 3in, whose leader line text is \"x\". ");

return document;
