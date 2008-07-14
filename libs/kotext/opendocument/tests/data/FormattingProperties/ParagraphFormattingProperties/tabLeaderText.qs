include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
var tabstop = new KoTextTab;
tabstop.position = 3 * 72; // 1 in = 72 pts

tabstop.leaderType = KoCharacterStyle.SingleLine;
tabstop.leaderStyle = KoCharacterStyle.SolidLine;
tabstop.leaderText = "$";
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of paragraph with tab stop at 3in, whose leader line text is \"$\".");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderType = KoCharacterStyle.SingleLine;
tabstop.leaderStyle = KoCharacterStyle.SolidLine;
tabstop.leaderText = "@";
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of paragraph with tab stop at 3in, whose leader line text is \"@\".");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderType = KoCharacterStyle.SingleLine;
tabstop.leaderStyle = KoCharacterStyle.SolidLine;
tabstop.leaderText = "x";
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("this is (tab)\tan example of paragraph with tab stop at 3in, whose leader line text is \"x\".");

return document;

// Actually, you dont need to set leaderType and leaderStyle for leaderText to take effect in KWord
// But in OO.o, you need leaderStyle not to be none. So, to make the example odts openable in OO.o,
// we made the odts set leaderStyle.
