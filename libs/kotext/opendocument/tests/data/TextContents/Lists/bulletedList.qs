include("common.qs");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListCircle);
setFormatProperty(listFormat, KoListStyle.BulletCharacter, 0x2022);
cursor.createList(listFormat);
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);

return document;
