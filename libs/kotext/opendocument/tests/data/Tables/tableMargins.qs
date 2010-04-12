/*
 * Current issues with this test:
 *
 *  - Support for num-rows-repeated.
 *  - Fix empty block at end of table.
 */

include("common.qs");

// 0.1 inch margin.
var firstTableFormat = QTextTableFormat.clone(defaultTableFormat);
firstTableFormat.setTopMargin(7.2); // 7.2 pt == 0.1 in
firstTableFormat.setBottomMargin(7.2); 
firstTableFormat.setLeftMargin(7.2); 
firstTableFormat.setRightMargin(7.2); 

// 0.5 inch margin.
var secondTableFormat = QTextTableFormat.clone(defaultTableFormat);
secondTableFormat.setTopMargin(36); // 36 pt == 0.5 in
secondTableFormat.setBottomMargin(36); 
secondTableFormat.setLeftMargin(36); 
secondTableFormat.setRightMargin(36); 

// 1 inch margin.
var thirdTableFormat = QTextTableFormat.clone(defaultTableFormat);
thirdTableFormat.setTopMargin(72); // 72 pt == 1 in
thirdTableFormat.setBottomMargin(72);
thirdTableFormat.setLeftMargin(72);
thirdTableFormat.setRightMargin(72);

// first table
cursor.insertText("this is an example of table with the margin of 0.1 inch.", defaultTextFormat);
cursor.insertTable(1, 3, firstTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// second table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with the margin of 0.5 inch.", defaultTextFormat);
cursor.insertTable(1, 3, secondTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// third table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table with the margin of 1 inch.", defaultTextFormat);
cursor.insertTable(1, 3, thirdTableFormat);

cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

document;
