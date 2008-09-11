include("common.qs");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListDecimal);
setFormatProperty(listFormat, KoListStyle.ListItemSuffix, ".");

var headerFormat = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(headerFormat, KoParagraphStyle.IsListHeader, 1);

var list = cursor.createList(listFormat);
cursor.mergeBlockFormat(headerFormat);
cursor.insertText("This is an example of list header.", defaultListItemFormat);

cursor.insertBlock();
cursor.mergeBlockFormat(headerFormat);
cursor.insertText("This is an example of list header.", defaultListItemFormat);

cursor.insertBlock();
cursor.mergeBlockFormat(headerFormat);
cursor.insertText("This is an example of list header.", defaultListItemFormat);

return document;
