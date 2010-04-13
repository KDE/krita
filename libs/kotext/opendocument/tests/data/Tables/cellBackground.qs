/*
 * Current issues with this test:
 *
 *  - Support for num-rows-repeated.
 *  - Fix empty block at end of table.
 */

include("common.qs");
var tableCellFormat = QTextCharFormat.clone(defaultTextFormat);
tableCellFormat.setProperty(KoTableCellStyle.CellBackgroundBrush, new QBrush(new QColor(0xff, 0x33, 0x66)));
cursor.insertText("this is an example of table with a cell background color of red.", defaultTextFormat);
var table = cursor.insertTable(1, 3, defaultTableFormat);
var cell = table.cellAt(0,0)
cell.setFormat(tableCellFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);
document;
