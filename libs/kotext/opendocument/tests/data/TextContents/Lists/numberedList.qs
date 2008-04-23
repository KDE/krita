include("common.qs");

cursor.createList(QTextListFormat.ListDecimal);
cursor.insertText("This is an example of numbered list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of numbered list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of numbered list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of numbered list.", defaultListItemFormat);

return document;
