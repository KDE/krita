include("common.qs");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListDecimal);
setFormatProperty(listFormat, KoListStyle.ListItemSuffix, ".");
setFormatProperty(listFormat, KoListStyle.StartValue, 1);
var list = cursor.createList(listFormat);
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock();
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock();
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock();
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("ready to continue");

cursor.insertBlock();
cursor.insertText("ready to continue");

cursor.insertBlock();
cursor.insertText("ready to continue");

var listFormat2 = QTextListFormat.clone(listFormat);
setFormatProperty(listFormat2, KoListStyle.ContinueNumbering, true);
var list2 = cursor.insertList(listFormat2);
cursor.insertText("continue numbering");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("continue numbering");
list2.add(cursor.block());

cursor.insertBlock();
cursor.insertText("continue numbering");

cursor.insertBlock();
cursor.insertText("continue numbering");

document;
