/*
 * Current issues with this test:
 *
 *  - Support for num-rows-repeated.
 *  - Fix empty block at end of table.
 */

include("common.qs");

var firstTableFormat = QTextTableFormat.clone(defaultTableFormat);
firstTableFormat.setProperty(KoTableStyle.CollapsingBorders, true);

var i = KoTableStyle.CollapsingBorders + 1;
print("The id is ", i);

// first table
cursor.insertText("this is an example of table with collapsing border model.", defaultTextFormat);
cursor.insertTable(1, 3, firstTableFormat);
cursor.movePosition(QTextCursor.End);
cursor.insertBlock(defaultBlockFormat);
document;
