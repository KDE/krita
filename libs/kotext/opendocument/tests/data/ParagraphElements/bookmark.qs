include("common.qs");

var bookmarkFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(bookmarkFormat, QTextFormat.ObjectType, QTextFormat.UserObject + 1);
setFormatProperty(bookmarkFormat, 577297549, 0);

var bookmarkStartFormat = bookmarkFormat;
var bookmarkEndFormat = bookmarkFormat;

cursor.insertText("Bookmarks can mark characters like this ", defaultTextFormat);
cursor.insertText(InlineObjectMaker, bookmarkFormat);
cursor.insertText("one.");

cursor.insertBlock();
cursor.insertText("Bookmarks can mark a range like this ", defaultTextFormat);
cursor.insertText(InlineObjectMaker, bookmarkStartFormat);
cursor.insertText("one", defaultTextFormat);
cursor.insertText(InlineObjectMaker, bookmarkEndFormat);
cursor.insertText(".", defaultTextFormat);

cursor.insertBlock();
cursor.insertText("Bookmarks can also span ", defaultTextFormat);
cursor.insertText(InlineObjectMaker, bookmarkStartFormat);
cursor.insertText("paragraphs", defaultTextFormat);
cursor.insertBlock();
cursor.insertText("like this ", defaultTextFormat);
cursor.insertText(InlineObjectMaker, bookmarkEndFormat);
cursor.insertText("one.", defaultTextFormat);

return document;
