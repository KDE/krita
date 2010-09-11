include("common.qs");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListDisc);
setFormatProperty(listFormat, KoListStyle.BulletCharacter, 0x2022);
setFormatProperty(listFormat, KoListStyle.MinimumWidth, 18);
setFormatProperty(listFormat, KoListStyle.Indent, 18);
cursor.createList(listFormat);
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);

document;
