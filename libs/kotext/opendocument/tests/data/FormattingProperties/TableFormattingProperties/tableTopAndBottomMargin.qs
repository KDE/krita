/*
 * Current issues with this test:
 *
 *  - Support for num-rows-repeated.
 *  - Fix empty block at end of table.
 */

include("common.qs");

// 0.1 inch top margin and 0.4 inch bottom margin.
var firstTableFormat = QTextTableFormat.clone(defaultTableFormat);
firstTableFormat.setTopMargin(7.2); // 7.2 pt == 0.1 in
firstTableFormat.setBottomMargin(28.8); // 28.8 pt == 0.4 in

// 0.4 inch top margin and 0.1 inch bottom margin.
var secondTableFormat = QTextTableFormat.clone(defaultTableFormat);
secondTableFormat.setTopMargin(28.8); // 28.8 pt == 0.4 in
secondTableFormat.setBottomMargin(7.2); // 7.2 pt == 0.1 in

// 0.4 inch top margin and 0.4 inch bottom margin.
var thirdTableFormat = QTextTableFormat.clone(defaultTableFormat);
thirdTableFormat.setTopMargin(28.8); // 28.8 pt == 0.4 in
thirdTableFormat.setBottomMargin(28.8); // 28.8 pt == 0.4 in

// first table
cursor.insertText("this is an example of table with the top margin of 0.1 inch and bottom margin of 0.4 inch.", defaultTextFormat);
cursor.insertTable(1, 3, firstTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// second table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with the top margin of 0.4 inch and bottom margin of 0.1 inch.", defaultTextFormat);
cursor.insertTable(1, 3, secondTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// third table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with the top margin of 0.4 inch and bottom margin of 0.4 inch.", defaultTextFormat);
cursor.insertTable(1, 3, thirdTableFormat);

cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

document;
