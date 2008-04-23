include("common.qs");

var listFormat1 = QTextListFormat.clone(defaultListFormat);
listFormat1.setStyle(QTextListFormat.ListDisc);
var list1 = cursor.createList(listFormat1);
cursor.insertText("This is an example of embedded bulleted list", defaultListItemFormat);

var list2Format = new QTextListFormat.clone(defaultListFormat);
list2Format.setStyle(QTextListFormat.ListCircle);
//list2Format.setIndent(2); // why is this not requried?
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
