// this is very similar to boldAndItalic.qs

include("common.qs");

cursor.insertText("Foo");

cursor.insertBlock();
cursor.insertText("Foo bar");

cursor.insertBlock();
cursor.insertText("Foobar baz");

cursor.insertBlock();
cursor.insertText("Foo bar ");

cursor.insertBlock();
cursor.insertText("Foo  ");

cursor.insertBlock();
cursor.insertText("Foo  Bar");

cursor.insertBlock();
cursor.insertText("Foo ");

return document;
