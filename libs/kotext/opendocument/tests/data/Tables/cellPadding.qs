/*
 * Current issues with this test:
 *
 *  - Support for num-rows-repeated.
 *  - Fix empty block at end of table.
 */

include("common.qs");
cursor.insertText("this is an example of table with cell padding.", defaultTextFormat);
var table = cursor.insertTable(1, 5, defaultTableFormat);

var leftPaddingFormat = QTextCharFormat.clone(defaultTextFormat);
leftPaddingFormat.setProperty(QTextFormat.TableCellLeftPadding, 7.2);
table.cellAt(0,0).setFormat(leftPaddingFormat);

var rightPaddingFormat = QTextCharFormat.clone(defaultTextFormat);
rightPaddingFormat.setProperty(QTextFormat.TableCellRightPadding, 7.2);
table.cellAt(0,1).setFormat(rightPaddingFormat);

var topPaddingFormat = QTextCharFormat.clone(defaultTextFormat);
topPaddingFormat.setProperty(QTextFormat.TableCellTopPadding, 7.2);
table.cellAt(0,2).setFormat(topPaddingFormat);

var bottomPaddingFormat = QTextCharFormat.clone(defaultTextFormat);
bottomPaddingFormat.setProperty(QTextFormat.TableCellBottomPadding, 7.2);
table.cellAt(0,3).setFormat(bottomPaddingFormat);

var paddingFormat = QTextCharFormat.clone(defaultTextFormat);
paddingFormat.setProperty(QTextFormat.TableCellLeftPadding, 7.2);
paddingFormat.setProperty(QTextFormat.TableCellRightPadding, 7.2);
paddingFormat.setProperty(QTextFormat.TableCellTopPadding, 7.2);
paddingFormat.setProperty(QTextFormat.TableCellBottomPadding, 7.2);
table.cellAt(0,4).setFormat(paddingFormat);

cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);
document;
