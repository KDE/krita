include("common.qs");

var alignStartFormat = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(alignStartFormat, QTextFormat.BlockAlignment, 1 /*Qt.AlignLeading*/);

var alignEndFormat = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(alignEndFormat, QTextFormat.BlockAlignment, 2 /*Qt.AlignTrailing*/);

var alignLeftFormat = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(alignLeftFormat, QTextFormat.BlockAlignment, 17 /*Qt.AlignLeft | Qt.AlignAbsolute*/);

var alignRightFormat = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(alignRightFormat, QTextFormat.BlockAlignment, 18 /*Qt.AlignRight | Qt.AlignAbsolute*/);

var alignCenterFormat = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(alignCenterFormat, QTextFormat.BlockAlignment, 4 /*Qt.AlignHCenter*/);

var alignJustifyFormat = QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(alignJustifyFormat, QTextFormat.BlockAlignment, 8 /*Qt.AlignJustify*/);

cursor.setCharFormat(QTextCharFormat.clone(defaultTextFormat));
cursor.setBlockFormat(alignStartFormat);
cursor.insertText("This is an example of paragraph with text align of start.");
cursor.insertBlock(alignEndFormat);
cursor.insertText("This is an example of paragraph with text align of end.");
cursor.insertBlock(alignLeftFormat);
cursor.insertText("This is an example of paragraph with text align of left.");
cursor.insertBlock(alignRightFormat);
cursor.insertText("This is an example of paragraph with text align of right.");
cursor.insertBlock(alignCenterFormat);
cursor.insertText("This is an example of paragraph with text align of center.");
cursor.insertBlock(alignJustifyFormat);
cursor.insertText("This is an example of paragraph with text align of justify.");

return document;
