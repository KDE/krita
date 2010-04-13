/*
 * Current issues with this test:
 *
 *  - Support for num-rows-repeated.
 *  - Fix empty block at end of table.
 */

include("common.qs");

var firstTableFormat = QTextTableFormat.clone(defaultTableFormat);
firstTableFormat.setProperty(KoTableStyle.KeepWithNext, true);

var secondTableFormat = QTextTableFormat.clone(defaultTableFormat);
secondTableFormat.setProperty(KoTableStyle.KeepWithNext, true);

// first table
cursor.insertText("this is an example of table.", defaultTextFormat);
cursor.insertTable(1, 3, firstTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

// second table
cursor.movePosition(QTextCursor.End);
cursor.insertText("this is an example of table.", defaultTextFormat);
cursor.insertTable(1, 3, secondTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.insertBlock(defaultBlockFormat);
document;
