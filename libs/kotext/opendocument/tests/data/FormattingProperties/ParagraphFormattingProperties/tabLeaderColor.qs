include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
var textCharFormat = new QTextCharFormat.clone(defaultTextFormat);
textCharFormat.setForeground(new QBrush(new QColor("#ff3366")));
var tabstop = new KoTextTab;
tabstop.position = 3 * 72; // 1 in = 72 pts

tabstop.leaderColor = "font-color";
tabstop.leaderType = KoCharacterStyle.SingleLine;
tabstop.leaderStyle = KoCharacterStyle.SolidLine;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of paragraph with tab stop, whose leader line color is font color.", textCharFormat);
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderColor = "#3deb3d";
tabstop.leaderType = KoCharacterStyle.SingleLine;
tabstop.leaderStyle = KoCharacterStyle.SolidLine;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of paragraph with tab stop, whose leader line color is green.", textCharFormat);

return document;
