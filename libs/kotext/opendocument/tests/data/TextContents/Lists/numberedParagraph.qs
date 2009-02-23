include("common.qs");

cursor.insertText("Paragraph");

var listFormat = QTextListFormat.clone(defaultListFormat);
listFormat.setStyle(QTextListFormat.ListDecimal);
setFormatProperty(listFormat, KoListStyle.ListItemSuffix, ".");
setFormatProperty(listFormat, KoListStyle.StartValue, 1);
setFormatProperty(listFormat, KoListStyle.DisplayLevel, 1);

var level1Format = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(level1Format, KoParagraphStyle.ListLevel, 1);

var level2Format = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(level2Format, KoParagraphStyle.ListLevel, 2);

cursor.insertBlock(level1Format);
var list1 = cursor.createList(listFormat);
cursor.insertText("One", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("Two", defaultListItemFormat);

cursor.insertBlock(level2Format);
var listFormat2 = QTextListFormat.clone(listFormat);
setFormatProperty(listFormat2, KoListStyle.Level, 2);
setFormatProperty(listFormat2, KoListStyle.DisplayLevel, 2);
var list2 = cursor.createList(listFormat2);
cursor.insertText("Two.One", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("Two.Two", defaultListItemFormat);
cursor.insertBlock();
cursor.insertText("Two.Three", defaultListItemFormat);

cursor.insertBlock(level1Format);
list1.add(cursor.block());
cursor.insertText("Heading");

document;
