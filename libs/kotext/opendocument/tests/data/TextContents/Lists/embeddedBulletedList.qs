include("common.qs");

var list1 = cursor.createList(QTextListFormat.ListDisc);
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);

var list2Format = new QTextListFormat;
list2Format.setStyle(QTextListFormat.ListCircle);
list2Format.setIndent(2);
var list2 = cursor.insertList(list2Format);
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);

cursor.insertBlock();
list1.add(cursor.block());
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);

return document;
