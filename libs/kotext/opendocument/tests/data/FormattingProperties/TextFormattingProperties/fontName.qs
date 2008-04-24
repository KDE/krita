include("common.qs");

var arialBlackFormat = QTextCharFormat.clone(defaultTextFormat);
var arialBlackFont = arialBlackFormat.font();
arialBlackFont.setFamily("Arial Black");
arialBlackFormat.setFont(arialBlackFont);

var timesNewRomanFormat = QTextCharFormat.clone(defaultTextFormat);
var timesNewRomanFont = timesNewRomanFormat.font();
timesNewRomanFont.setFamily("Times New Roman");
timesNewRomanFormat.setFont(timesNewRomanFont);

cursor.insertText("This is an example of assign a font to the text. ", timesNewRomanFormat);
cursor.insertBlock();
cursor.insertText("This is an example of assign a font to the text. ", arialBlackFormat);

return document;
