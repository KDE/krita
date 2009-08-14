/*
 * Current issues with this test:
 *
 *  - Support for num-rows-repeated.
 *  - Fix empty block at end of table.
 */

include("common.qs");

// 1 inch left margin and 2 inch right margin.
var firstTableFormat = QTextTableFormat.clone(defaultTableFormat);
firstTableFormat.setLeftMargin(72.0); // 72 pt == 1 in
firstTableFormat.setRightMargin(144.0); // 144 pt == 2 in
firstTableFormat.setAlignment(Qt.AlignJustify);

// 2 inch left margin and 1 inch right margin.
var secondTableFormat = QTextTableFormat.clone(defaultTableFormat);
secondTableFormat.setLeftMargin(144.0); // 144 pt == 2 in
secondTableFormat.setRightMargin(72.0); // 72 pt == 1 in
secondTableFormat.setAlignment(Qt.AlignJustify);

// 2 inch left margin and 2 inch right margin.
var thirdTableFormat = QTextTableFormat.clone(defaultTableFormat);
thirdTableFormat.setLeftMargin(144.0); // 144 pt == 2 in
thirdTableFormat.setRightMargin(144.0); // 144 pt == 1 in
thirdTableFormat.setAlignment(Qt.AlignJustify);

// first table
cursor.insertText("this is an example of table with the left margin of 1 inch and right margin of 2 inch.", defaultTextFormat);
cursor.insertTable(1, 3, firstTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// second table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with the left margin of 2 inch and right margin of 1 inch.", defaultTextFormat);
cursor.insertTable(1, 3, secondTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// third table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with the left margin of 2 inch and right margin of 2 inch.", defaultTextFormat);
cursor.insertTable(1, 3, thirdTableFormat);

cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

document;
