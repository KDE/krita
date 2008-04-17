include("common.qs");

var redFormat = QTextCharFormat.clone(defaultTextFormat);
redFormat.setForeground(new QBrush(new QColor("#ff3366")));
cursor.insertText("this is an example of specifying the foreground color of the text.", redFormat);

cursor.insertText(" "); // CHECKME: Why is this appended?

cursor.insertBlock(new QTextBlockFormat, new QTextCharFormat); // CHECKME: I don't think this should be created

return document;
