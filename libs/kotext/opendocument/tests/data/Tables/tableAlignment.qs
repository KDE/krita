/*
 * Current issues with this test:
 *  - Fix empty block at end of table.
 */

include("common.qs");

// left aligned table
var leftAlignTableFormat = QTextTableFormat.clone(defaultTableFormat);
leftAlignTableFormat.setAlignment(Qt.AlignLeft);

// center aligned table
var centerAlignTableFormat = QTextTableFormat.clone(defaultTableFormat);
centerAlignTableFormat.setAlignment(Qt.AlignHCenter);

// right aligned table
var rightAlignTableFormat = QTextTableFormat.clone(defaultTableFormat);
rightAlignTableFormat.setAlignment(Qt.AlignRight);

// margin aligned table
var marginsAlignTableFormat = QTextTableFormat.clone(defaultTableFormat);
marginsAlignTableFormat.setAlignment(Qt.AlignJustify);

// first table
cursor.insertText("this is an example of table with the left alignment.", defaultTextFormat);
cursor.insertTable(1, 3, leftAlignTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// second table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with the right alignment.", defaultTextFormat);
cursor.insertTable(1, 3, rightAlignTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// third table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with the center alignment.", defaultTextFormat);
cursor.insertTable(1, 3, centerAlignTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// third table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with the margins alignment.", defaultTextFormat);
cursor.insertTable(1, 3, marginsAlignTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

document;
