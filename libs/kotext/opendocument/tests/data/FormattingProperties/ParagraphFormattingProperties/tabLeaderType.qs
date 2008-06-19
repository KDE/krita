include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
var tabstop = new KoTextTab;
tabstop.position = 1 * 72; // 1 in = 72 pts

tabstop.leaderType = KoCharacterStyle.NoLineType;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("(tab)\twithout a leader line.");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderType = KoCharacterStyle.SingleLine;
tabstop.leaderStyle = KoCharacterStyle.SolidLine;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("(tab)\twith a single leader line.");
cursor.insertBlock(defaultBlockFormat);

tabstop.leaderType = KoCharacterStyle.DoubleLine;
tabstop.leaderStyle = KoCharacterStyle.SolidLine;
setFormatProperty(textBlockFormat, KoParagraphStyle.TabPositions, [tabstop]);
cursor.setBlockFormat(textBlockFormat);
cursor.insertText("(tab)\twith a double leader line.");

return document;
