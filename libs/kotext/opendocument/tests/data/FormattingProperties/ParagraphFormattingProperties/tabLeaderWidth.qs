include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
var tabstop = new KoTextTab;
tabstop.position = 3 * 72; // 1 in = 72 pts
tabstop.leaderStyle = KoCharacterStyle.SolidLine;
tabstop.leaderWeight = KoCharacterStyle.NormalLineWeight;

setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of paragraph with tab stop at 3in, whose leader line width is normal. ");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderWeight = KoCharacterStyle.BoldLineWeight;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of paragraph with tab stop at 3in, whose leader line width is bold. ");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderWeight = KoCharacterStyle.ThinLineWeight;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of paragraph with tab stop at 3in, whose leader line width is thin. ");

return document;
