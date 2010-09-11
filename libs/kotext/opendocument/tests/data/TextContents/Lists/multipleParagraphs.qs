include("common.qs");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListDisc);
setFormatProperty(listFormat, KoListStyle.BulletCharacter, 0x2022);
var list = cursor.createList(listFormat);
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);

var unnumberedFormat = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(unnumberedFormat, KoParagraphStyle.UnnumberedListItem, 1);
cursor.insertBlock();
cursor.mergeBlockFormat(unnumberedFormat);
cursor.insertText("This item must not be bulleted.", defaultListItemFormat);

cursor.insertBlock();
cursor.mergeBlockFormat(unnumberedFormat);
cursor.insertText("This item must not be bulleted.", defaultListItemFormat);

cursor.insertBlock(defaultBlockFormat);
list.add(cursor.block());
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);

cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.", defaultListItemFormat);

document;
