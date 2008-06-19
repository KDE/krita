include("common.qs");

var redFormat = QTextCharFormat.clone(defaultTextFormat);
redFormat.setForeground(new QBrush(new QColor("#ff3366")));
cursor.insertText("this is an example of specifying the foreground color of the text.", redFormat);

return document;
