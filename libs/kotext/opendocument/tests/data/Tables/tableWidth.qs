/*
 * Current issues with this test:
 *  - Fix empty block at end of table.
 */

include("common.qs");

// 2 inch wide table
var twoInchTableFormat = QTextTableFormat.clone(defaultTableFormat);
twoInchTableFormat.setWidth(new QTextLength(QTextLength.FixedLength, 144.0)); // 144 pt == 2 in

// 5 inch wide table
var fiveInchTableFormat = QTextTableFormat.clone(defaultTableFormat);
fiveInchTableFormat.setWidth(new QTextLength(QTextLength.FixedLength, 360.0)); // 360 pt == 5 in

// 75 % wide table
var seventyFivePercentTableFormat = QTextTableFormat.clone(defaultTableFormat);
seventyFivePercentTableFormat.setWidth(new QTextLength(QTextLength.PercentageLength, 75.0));

// first table
cursor.insertText("this is an example of table with fixed table width of 2 inch.", defaultTextFormat);
cursor.insertTable(1, 3, twoInchTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// second table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with fixed table width of 5 inch.", defaultTextFormat);
cursor.insertTable(1, 3, fiveInchTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// third table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with relative table width of 75%.", defaultTextFormat);
cursor.insertTable(1, 3, seventyFivePercentTableFormat);

cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

document;
