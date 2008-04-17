// include("common.qs");

cursor.insertList(QTextListFormat.ListDisc);
cursor.insertText("This is an example of bulleted list.");
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.");
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.");
cursor.insertBlock();
cursor.insertText("This is an example of bulleted list.");

return document;
