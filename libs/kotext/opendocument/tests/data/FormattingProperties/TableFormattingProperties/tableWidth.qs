/*
 * Current issues with this test:
 *
 *  - Support for num-rows-repeated.
 *  - Fix empty block at end of table.
 */

include("common.qs");

var twoInchTableFormat = QTextTableFormat.clone(defaultTableFormat);
twoInchTableFormat.setWidth(new QTextLength(QTextLength.FixedLength, 144.0));
twoInchTableFormat.setAlignment(Qt.AlignLeft);

var fiveInchTableFormat = QTextTableFormat.clone(defaultTableFormat);
fiveInchTableFormat.setWidth(new QTextLength(QTextLength.FixedLength, 360.0));
fiveInchTableFormat.setAlignment(Qt.AlignLeft);

var seventyFivePercentTableFormat = QTextTableFormat.clone(defaultTableFormat);
seventyFivePercentTableFormat.setWidth(new QTextLength(QTextLength.PercentageLength, 75.0));
seventyFivePercentTableFormat.setAlignment(Qt.AlignLeft);

cursor.insertText("this is an example of table with fixed table width of 2 inch.", defaultTextFormat);
cursor.insertTable(1, 3, twoInchTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with fixed table width of 5 inch.", defaultTextFormat);
cursor.insertTable(1, 3, fiveInchTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with relative table width of 75%.", defaultTextFormat);
cursor.insertTable(1, 3, seventyFivePercentTableFormat);

cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

document;
