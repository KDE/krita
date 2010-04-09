/*
 * Current issues with this test:
 *  - Fix empty block at end of table.
 */

include("common.qs");

cursor.insertText("this is an example of a simple table containing a row-spanning cell", defaultTextFormat);
cursor.insertTable(3, 3, defaultTableFormat);
cursor.currentTable().mergeCells(1,0,2,1);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.insertText("this is an example of a simple table containing a column-spanning cell", defaultTextFormat);
cursor.insertTable(3, 3, defaultTableFormat);
cursor.currentTable().mergeCells(1,0,1,2);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.insertText("this is an example of a simple table containing a row and column-spanning cell", defaultTextFormat);
cursor.insertTable(3, 3, defaultTableFormat);
cursor.currentTable().mergeCells(1,0,2,2);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);

cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
document;
