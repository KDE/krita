include("common.qs");

var twentyEightPointFormat = QTextCharFormat.clone(defaultTextFormat);
var twentyEightPointFont = new QFont(defaultFont);
twentyEightPointFont.setPointSizeF(28.0);
twentyEightPointFormat.setForeground(new QBrush(new QColor(0, 0, 0)));
twentyEightPointFormat.setFont(twentyEightPointFont);

cursor.insertText("The following is a character whose font size is ", defaultTextFormat);
cursor.insertText("28.", twentyEightPointFormat);
cursor.insertText(" ", defaultTextFormat); // CHECKME: Why is a blank space appended?

cursor.insertBlock(); // CHECKME: This should not be created

return document;
