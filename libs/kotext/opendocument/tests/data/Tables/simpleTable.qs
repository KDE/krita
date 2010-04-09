/*
 * Current issues with this test:
 *  - Fix empty block at end of table.
 */

include("common.qs");

cursor.insertText("this is an example of a simple table containing 1 row and 1 column", defaultTextFormat);
cursor.insertTable(1, 1, defaultTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.insertText("this is an example of a simple table containing 1 row and 3 columns", defaultTextFormat);
cursor.insertTable(1, 3, defaultTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.insertText("this is an example of a simple table containing 3 rows and 1 column", defaultTextFormat);
cursor.insertTable(3, 1, defaultTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.insertText("this is an example of a simple table containing 3 rows and 3 columns", defaultTextFormat);
cursor.insertTable(3, 3, defaultTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);

document;
