include("common.qs");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListDecimal);
setFormatProperty(listFormat, KoListStyle.ListItemSuffix, ".");
var list = cursor.createList(listFormat);
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock();
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock();
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock();
cursor.insertText("how are you doing.", defaultListItemFormat);

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("ready to restart from 80.");

cursor.insertBlock();
cursor.insertText("ready to restart from 80.");

cursor.insertBlock();
cursor.insertText("ready to restart from 80.");

var restartFormat = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(restartFormat, KoParagraphStyle.ListStartValue, 80);
cursor.insertBlock(restartFormat);
list.add(cursor.block());
cursor.insertText("test start-value for list");

cursor.insertBlock(defaultBlockFormat);
cursor.insertText("test start-value for list");
list.add(cursor.block());

cursor.insertBlock();
cursor.insertText("test start-value for list");

cursor.insertBlock();
cursor.insertText("test start-value for list");

return document;
