include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
var tabstop = new KoTextTab;
var tabstop2 = new KoTextTab;
tabstop.position = 3 * 72; // 1 in = 72 pts
tabstop2.position = 6 * 72; // 1 in = 72 pts

tabstop.leaderType = KoCharacterStyle.SingleLine;
tabstop.leaderStyle = KoCharacterStyle.DottedLine;
tabstop2.leaderType = KoCharacterStyle.SingleLine;
tabstop2.leaderStyle = KoCharacterStyle.WaveLine;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop, tabstop2]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of (tab)\tparagraph with tab stop at 3in/6in, whose first leader line style is dotted and the second is wave.");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderType = KoCharacterStyle.SingleLine;
tabstop.leaderStyle = KoCharacterStyle.SolidLine;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of (tab)\tparagraph with tab stop at 3in, whose leader line style is solid.");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderType = KoCharacterStyle.SingleLine;
tabstop.leaderStyle = KoCharacterStyle.WaveLine;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of (tab)\tparagraph with tab stop at 3in, whose leader line style is wave.");

return document;
