include("common.qs");

var listFormat1 = QTextListFormat.clone(defaultListFormat);
setFormatProperty(listFormat1, QTextFormat.ListStyle, KoListStyle.BlackCircle);
setFormatProperty(listFormat1, KoListStyle.BulletCharacter, 0x25CF);
setFormatProperty(listFormat1, KoListStyle.MinimumWidth, 18);
setFormatProperty(listFormat1, KoListStyle.Indent, 18);
setFormatProperty(listFormat1, KoListStyle.RelativeBulletSize, 45);
var list1 = cursor.createList(listFormat1);
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);

var list2Format = new QTextListFormat.clone(defaultListFormat);
setFormatProperty(list2Format, QTextFormat.ListStyle, KoListStyle.Bullet);
setFormatProperty(list2Format, KoListStyle.BulletCharacter, 0x2022);
setFormatProperty(list2Format, KoListStyle.MinimumWidth, 18);
setFormatProperty(list2Format, KoListStyle.Indent, 18);
setFormatProperty(list2Format, KoListStyle.RelativeBulletSize, 45);
var list2 = cursor.insertList(list2Format);
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);

cursor.insertBlock();
list1.add(cursor.block());
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);

document;
