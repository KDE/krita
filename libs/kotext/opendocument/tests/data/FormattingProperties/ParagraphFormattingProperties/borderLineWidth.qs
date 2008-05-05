// include("common.qs");

var textBlockFormat = new QTextBlockFormat.clone(defaultBlockFormat);
setFormatProperty(textBlockFormat, KoParagraphStyle.LeftBorderWidth, 0.0154 * 72);
setFormatProperty(textBlockFormat, KoParagraphStyle.TopBorderWidth, 0.0154 * 72);
setFormatProperty(textBlockFormat, KoParagraphStyle.RightBorderWidth, 0.0154 * 72);
setFormatProperty(textBlockFormat, KoParagraphStyle.BottomBorderWidth, 0.0154 * 72);
setFormatProperty(textBlockFormat, KoParagraphStyle.LeftBorderStyle, KoParagraphStyle.BorderDouble);
setFormatProperty(textBlockFormat, KoParagraphStyle.TopBorderStyle, KoParagraphStyle.BorderDouble);
setFormatProperty(textBlockFormat, KoParagraphStyle.RightBorderStyle, KoParagraphStyle.BorderDouble);
setFormatProperty(textBlockFormat, KoParagraphStyle.BottomBorderStyle, KoParagraphStyle.BorderDouble);
setFormatProperty(textBlockFormat, KoParagraphStyle.LeftBorderColor, new QColor("#000000"));
setFormatProperty(textBlockFormat, KoParagraphStyle.TopBorderColor, new QColor("#000000"));
setFormatProperty(textBlockFormat, KoParagraphStyle.RightBorderColor, new QColor("#000000"));
setFormatProperty(textBlockFormat, KoParagraphStyle.BottomBorderColor, new QColor("#000000"));

var textBlockFormatAllSides = new QTextBlockFormat.clone(textBlockFormat);
var innerBorderWidthIds = [KoParagraphStyle.LeftInnerBorderWidth, KoParagraphStyle.TopInnerBorderWidth,
   KoParagraphStyle.RightInnerBorderWidth, KoParagraphStyle.BottomInnerBorderWidth];
for(var i = 0; i < innerBorderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, innerBorderWidthIds[i], 0.01 * 72);
}
var borderSpacingIds = [KoParagraphStyle.LeftBorderSpacing, KoParagraphStyle.TopBorderSpacing,
   KoParagraphStyle.RightBorderSpacing, KoParagraphStyle.BottomBorderSpacing];
for(var i = 0; i < borderSpacingIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderSpacingIds[i], 0.02 * 72);
}
var borderWidthIds = [KoParagraphStyle.LeftBorderWidth, KoParagraphStyle.TopBorderWidth,
   KoParagraphStyle.RightBorderWidth, KoParagraphStyle.BottomBorderWidth];
for(var i = 0; i < borderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderWidthIds[i], 0.01 * 72);
}

cursor.setBlockFormat(textBlockFormatAllSides);
cursor.insertText("This is an example of paragraph with double border. The width of inner line is 0.01 inch, the distance between two line is 0.02 inch, and the width of the outer line is 0.01 inch. "); // P1
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);


var innerBorderWidthIds = [KoParagraphStyle.LeftInnerBorderWidth, KoParagraphStyle.TopInnerBorderWidth,
   KoParagraphStyle.RightInnerBorderWidth, KoParagraphStyle.BottomInnerBorderWidth];
for(var i = 0; i < innerBorderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, innerBorderWidthIds[i], 0.03 * 72);
}
var borderSpacingIds = [KoParagraphStyle.LeftBorderSpacing, KoParagraphStyle.TopBorderSpacing,
   KoParagraphStyle.RightBorderSpacing, KoParagraphStyle.BottomBorderSpacing];
for(var i = 0; i < borderSpacingIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderSpacingIds[i], 0.02 * 72);
}
var borderWidthIds = [KoParagraphStyle.LeftBorderWidth, KoParagraphStyle.TopBorderWidth,
   KoParagraphStyle.RightBorderWidth, KoParagraphStyle.BottomBorderWidth];
for(var i = 0; i < borderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderWidthIds[i], 0.01 * 72);
}

cursor.setBlockFormat(textBlockFormatAllSides);
cursor.insertText("This is an example of paragraph with double border. The width of inner line is 0.03 inch, the distance between two line is 0.02 inch, and the width of the outer line is 0.01 inch. "); // P2
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);


var innerBorderWidthIds = [KoParagraphStyle.LeftInnerBorderWidth, KoParagraphStyle.TopInnerBorderWidth,
   KoParagraphStyle.RightInnerBorderWidth, KoParagraphStyle.BottomInnerBorderWidth];
for(var i = 0; i < innerBorderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, innerBorderWidthIds[i], 0.01 * 72);
}
var borderSpacingIds = [KoParagraphStyle.LeftBorderSpacing, KoParagraphStyle.TopBorderSpacing,
   KoParagraphStyle.RightBorderSpacing, KoParagraphStyle.BottomBorderSpacing];
for(var i = 0; i < borderSpacingIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderSpacingIds[i], 0.02 * 72);
}
var borderWidthIds = [KoParagraphStyle.LeftBorderWidth, KoParagraphStyle.TopBorderWidth,
   KoParagraphStyle.RightBorderWidth, KoParagraphStyle.BottomBorderWidth];
for(var i = 0; i < borderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderWidthIds[i], 0.03 * 72);
}

cursor.setBlockFormat(textBlockFormatAllSides);
cursor.insertText("This is an example of paragraph with double border. The width of inner line is 0.01 inch, the distance between two line is 0.02 inch, and the width of the outer line is 0.03 inch. "); // P3
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);



var innerBorderWidthIds = [KoParagraphStyle.LeftInnerBorderWidth, KoParagraphStyle.TopInnerBorderWidth,
   KoParagraphStyle.RightInnerBorderWidth, KoParagraphStyle.BottomInnerBorderWidth];
for(var i = 0; i < innerBorderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, innerBorderWidthIds[i], 0.01 * 72);
}
var borderSpacingIds = [KoParagraphStyle.LeftBorderSpacing, KoParagraphStyle.TopBorderSpacing,
   KoParagraphStyle.RightBorderSpacing, KoParagraphStyle.BottomBorderSpacing];
for(var i = 0; i < borderSpacingIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderSpacingIds[i], 0.4 * 72);
}
var borderWidthIds = [KoParagraphStyle.LeftBorderWidth, KoParagraphStyle.TopBorderWidth,
   KoParagraphStyle.RightBorderWidth, KoParagraphStyle.BottomBorderWidth];
for(var i = 0; i < borderWidthIds.length; i++) {
  setFormatProperty(textBlockFormatAllSides, borderWidthIds[i], 0.01 * 72);
}

cursor.setBlockFormat(textBlockFormatAllSides);
cursor.insertText("This is an example of paragraph with double border. The width of inner line is 0.01 inch, the distance between two line is 0.4 inch, and the width of the outer line is 0.01 inch. "); // P4
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);
cursor.insertBlock(defaultBlockFormat);



var textBlockFormatLeftOnly = new QTextBlockFormat.clone(textBlockFormat);
setFormatProperty(textBlockFormatLeftOnly, KoParagraphStyle.LeftInnerBorderWidth, 0.01 * 72);
setFormatProperty(textBlockFormatLeftOnly, KoParagraphStyle.LeftBorderSpacing, 0.3 * 72);
setFormatProperty(textBlockFormatLeftOnly, KoParagraphStyle.LeftBorderWidth, 0.03 * 72);

cursor.setBlockFormat(textBlockFormatLeftOnly);
cursor.insertText("This is an example of paragraph with double border. The width of inner line in left side is 0.01 inch, the distance between two line in left side is 0.3 inch, and the width of the outer line in left side is 0.03 inch. "); // P5

return document;
