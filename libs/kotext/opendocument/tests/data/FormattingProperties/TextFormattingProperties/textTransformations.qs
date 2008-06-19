include("common.qs");

var uppercaseFormat = QTextCharFormat.clone(defaultTextFormat);
uppercaseFormat.setFontCapitalization(QFont.AllUppercase);

var lowercaseFormat = QTextCharFormat.clone(defaultTextFormat);
lowercaseFormat.setFontCapitalization(QFont.AllLowercase);

var capitalizeFormat = QTextCharFormat.clone(defaultTextFormat);
capitalizeFormat.setFontCapitalization(QFont.Capitalize);

cursor.insertText("this is an example of text transformation to lower case.", lowercaseFormat);
cursor.insertBlock();
cursor.insertText("this is an example of text transformation to upper case.", uppercaseFormat);
cursor.insertBlock();
cursor.insertText("this is an example of text transformation to capitalize.", capitalizeFormat);

return document;
