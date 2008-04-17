include("common.qs");

var font = defaultTextFormat.font();
font.setFamily("Times New Roman");

var fixedPitchFormat = QTextCharFormat.clone(defaultTextFormat);
font.setFixedPitch(true);
fixedPitchFormat.setFont(font);

var variablePitchFormat = QTextCharFormat.clone(defaultTextFormat);
font.setFixedPitch(false);
variablePitchFormat.setFont(font);

cursor.insertText("This is an example that set font pitch to fixed. This is an example that set font pitch to fixed. This is an example that set font pitch to fixed.", fixedPitchFormat);
cursor.insertText(" ", defaultTextFormat);

cursor.insertBlock();
cursor.insertText("This is an example that set font pitch to variable. This is an example that set font pitch to variable. This is an example that set font pitch to variable.", variablePitchFormat);
cursor.insertText(" ", defaultTextFormat);

cursor.insertBlock(new QTextBlockFormat, new QTextCharFormat); // CHECKME: I don't think this should be created

return document;
