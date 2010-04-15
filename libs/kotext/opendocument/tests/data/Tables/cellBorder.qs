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
outerPen.setWidthF(7.2);

var innerPen = new QPen();
innerPen.setColor(new QColor(0xff, 0x33, 0x66));
innerPen.setJoinStyle(Qt.MiterJoin);
innerPen.setCapStyle(Qt.FlatCap);
innerPen.setStyle(Qt.SolidLine);
innerPen.setWidthF(0.0);

cursor.insertText("this is an example of table with cell border.", defaultTextFormat);
var table = cursor.insertTable(1, 7, defaultTableFormat);

var leftFormat = QTextCharFormat.clone(defaultTextFormat);
leftFormat.setProperty(KoTableCellStyle.LeftBorderOuterPen, outerPen);
leftFormat.setProperty(KoTableCellStyle.LeftBorderInnerPen, innerPen);
leftFormat.setProperty(KoTableCellStyle.LeftBorderSpacing, 0.0);
table.cellAt(0,0).setFormat(leftFormat);

var rightFormat = QTextCharFormat.clone(defaultTextFormat);
rightFormat.setProperty(KoTableCellStyle.RightBorderOuterPen, outerPen);
rightFormat.setProperty(KoTableCellStyle.RightBorderInnerPen, innerPen);
rightFormat.setProperty(KoTableCellStyle.RightBorderSpacing, 0.0);
table.cellAt(0,1).setFormat(rightFormat);

var topFormat = QTextCharFormat.clone(defaultTextFormat);
topFormat.setProperty(KoTableCellStyle.TopBorderOuterPen, outerPen);
topFormat.setProperty(KoTableCellStyle.TopBorderInnerPen, innerPen);
topFormat.setProperty(KoTableCellStyle.TopBorderSpacing, 0.0);
table.cellAt(0,2).setFormat(topFormat);

var bottomFormat = QTextCharFormat.clone(defaultTextFormat);
bottomFormat.setProperty(KoTableCellStyle.BottomBorderOuterPen, outerPen);
bottomFormat.setProperty(KoTableCellStyle.BottomBorderInnerPen, innerPen);
bottomFormat.setProperty(KoTableCellStyle.BottomBorderSpacing, 0.0);
table.cellAt(0,3).setFormat(bottomFormat);

var format = QTextCharFormat.clone(defaultTextFormat);
format.setProperty(KoTableCellStyle.LeftBorderOuterPen, outerPen);
format.setProperty(KoTableCellStyle.LeftBorderInnerPen, innerPen);
format.setProperty(KoTableCellStyle.LeftBorderSpacing, 0.0);
format.setProperty(KoTableCellStyle.RightBorderOuterPen, outerPen);
format.setProperty(KoTableCellStyle.RightBorderInnerPen, innerPen);
format.setProperty(KoTableCellStyle.RightBorderSpacing, 0.0);
format.setProperty(KoTableCellStyle.TopBorderOuterPen, outerPen);
format.setProperty(KoTableCellStyle.TopBorderInnerPen, innerPen);
format.setProperty(KoTableCellStyle.TopBorderSpacing, 0.0);
format.setProperty(KoTableCellStyle.BottomBorderOuterPen, outerPen);
format.setProperty(KoTableCellStyle.BottomBorderInnerPen, innerPen);
format.setProperty(KoTableCellStyle.BottomBorderSpacing, 0.0);
table.cellAt(0,4).setFormat(format);

cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);
document;
