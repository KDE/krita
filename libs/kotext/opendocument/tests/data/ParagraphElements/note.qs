include("common.qs");

var noteFormat = QTextCharFormat.clone(defaultTextFormat);
setFormatProperty(noteFormat, QTextFormat.ObjectType, QTextFormat.UserObject + 1);
setFormatProperty(noteFormat, 577297549, 0);

cursor.insertText("This paragraph contains a footnote", defaultTextFormat);
cursor.insertText(InlineObjectMaker, noteFormat);
cursor.insertText(".");

cursor.insertBlock();
cursor.insertText("This paragraph contains a footnote", defaultTextFormat);
cursor.insertText(InlineObjectMaker, noteFormat);
cursor.insertText(", too", defaultTextFormat);

return document;
