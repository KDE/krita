include("common.qs");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListDecimal);
setFormatProperty(listFormat, KoListStyle.ListItemSuffix, ".");
setFormatProperty(listFormat, KoListStyle.StartValue, 1);

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
cursor.insertBlock();
cursor.insertText("This is an example of multi-paragraph list header.", defaultListItemFormat);

document;
