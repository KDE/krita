/*
 * Current issues with this test:
 *
 *  - Support for num-rows-repeated.
 *  - Fix empty block at end of table.
 */

include("common.qs");
var outerPen = new QPen();
outerPen.setColor(new QColor(0xff, 0x33, 0x66));
outerPen.setJoinStyle(Qt.MiterJoin);
outerPen.setCapStyle(Qt.FlatCap);
outerPen.setStyle(Qt.SolidLine);
outerPen.setWidthF(288);

var innerPen = new QPen();
innerPen.setColor(new QColor(0xff, 0x33, 0x66));
innerPen.setJoinStyle(Qt.MiterJoin);
innerPen.setCapStyle(Qt.FlatCap);
innerPen.setStyle(Qt.SolidLine);
innerPen.setWidthF(288);

cursor.insertText("this is an example of table with cell border.", defaultTextFormat);
var table = cursor.insertTable(1, 4, defaultTableFormat);

var leftFormat = QTextCharFormat.clone(defaultTextFormat);
leftFormat.setProperty(KoTableCellStyle.LeftBorderOuterPen, outerPen);
leftFormat.setProperty(KoTableCellStyle.LeftBorderInnerPen, innerPen);
leftFormat.setProperty(KoTableCellStyle.LeftBorderSpacing, 144);
table.cellAt(0,0).setFormat(leftFormat);

var rightFormat = QTextCharFormat.clone(defaultTextFormat);
rightFormat.setProperty(KoTableCellStyle.RightBorderOuterPen, outerPen);
rightFormat.setProperty(KoTableCellStyle.RightBorderInnerPen, innerPen);
rightFormat.setProperty(KoTableCellStyle.RightBorderSpacing, 144);
table.cellAt(0,1).setFormat(rightFormat);

var topFormat = QTextCharFormat.clone(defaultTextFormat);
topFormat.setProperty(KoTableCellStyle.TopBorderOuterPen, outerPen);
topFormat.setProperty(KoTableCellStyle.TopBorderInnerPen, innerPen);
topFormat.setProperty(KoTableCellStyle.TopBorderSpacing, 144);
table.cellAt(0,2).setFormat(topFormat);

var bottomFormat = QTextCharFormat.clone(defaultTextFormat);
bottomFormat.setProperty(KoTableCellStyle.BottomBorderOuterPen, outerPen);
bottomFormat.setProperty(KoTableCellStyle.BottomBorderInnerPen, innerPen);
bottomFormat.setProperty(KoTableCellStyle.BottomBorderSpacing, 144);
table.cellAt(0,3).setFormat(bottomFormat);

cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);
document;
