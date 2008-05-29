var document = new QTextDocument;
var cursor = new QTextCursor(document);

// Default paragraph formatting
var KoParagraphStyle = {};
var QTextOption = {};
// enum KoParagraphStyle.Property
KoParagraphStyle.StyleId = QTextFormat.UserProperty + 1; // = 1048577
KoParagraphStyle.PercentLineHeight = QTextFormat.UserProperty + 2;
KoParagraphStyle.AutoTextIndent = QTextFormat.UserProperty + 52;
KoParagraphStyle.TabPositions = QTextFormat.UserProperty + 53;

// enum KoParagraphStyle.Property - border stuff
KoParagraphStyle.HasLeftBorder = QTextFormat.UserProperty + 17;
KoParagraphStyle.HasTopBorder = KoParagraphStyle.HasLeftBorder + 1;
KoParagraphStyle.HasRightBorder = KoParagraphStyle.HasLeftBorder + 2;
KoParagraphStyle.HasBottomBorder = KoParagraphStyle.HasLeftBorder + 3;
KoParagraphStyle.BorderLineWidth = KoParagraphStyle.HasLeftBorder + 4;
KoParagraphStyle.SecondBorderLineWidth = KoParagraphStyle.HasLeftBorder + 5;
KoParagraphStyle.DistanceToSecondBorder = KoParagraphStyle.HasLeftBorder + 6;
KoParagraphStyle.LeftPadding = KoParagraphStyle.HasLeftBorder + 7;
KoParagraphStyle.TopPadding = KoParagraphStyle.HasLeftBorder + 8;
KoParagraphStyle.RightPadding = KoParagraphStyle.HasLeftBorder + 9;
KoParagraphStyle.BottomPadding = KoParagraphStyle.HasLeftBorder + 10;
KoParagraphStyle.LeftBorderWidth = KoParagraphStyle.HasLeftBorder + 11;
KoParagraphStyle.LeftInnerBorderWidth = KoParagraphStyle.HasLeftBorder + 12;
KoParagraphStyle.LeftBorderSpacing = KoParagraphStyle.HasLeftBorder + 13;
KoParagraphStyle.LeftBorderStyle = KoParagraphStyle.HasLeftBorder + 14;
KoParagraphStyle.LeftBorderColor = KoParagraphStyle.HasLeftBorder + 15;
KoParagraphStyle.TopBorderWidth = KoParagraphStyle.HasLeftBorder + 16;
KoParagraphStyle.TopInnerBorderWidth = KoParagraphStyle.HasLeftBorder + 17;
KoParagraphStyle.TopBorderSpacing = KoParagraphStyle.HasLeftBorder + 18;
KoParagraphStyle.TopBorderStyle = KoParagraphStyle.HasLeftBorder + 19;
KoParagraphStyle.TopBorderColor = KoParagraphStyle.HasLeftBorder + 20;
KoParagraphStyle.RightBorderWidth = KoParagraphStyle.HasLeftBorder + 21;
KoParagraphStyle.RightInnerBorderWidth = KoParagraphStyle.HasLeftBorder + 22;
KoParagraphStyle.RightBorderSpacing = KoParagraphStyle.HasLeftBorder + 23;
KoParagraphStyle.RightBorderStyle = KoParagraphStyle.HasLeftBorder + 24;
KoParagraphStyle.RightBorderColor = KoParagraphStyle.HasLeftBorder + 25;
KoParagraphStyle.BottomBorderWidth = KoParagraphStyle.HasLeftBorder + 26;
KoParagraphStyle.BottomInnerBorderWidth = KoParagraphStyle.HasLeftBorder + 27;
KoParagraphStyle.BottomBorderSpacing = KoParagraphStyle.HasLeftBorder + 28;
KoParagraphStyle.BottomBorderStyle = KoParagraphStyle.HasLeftBorder + 29;
KoParagraphStyle.BottomBorderColor = KoParagraphStyle.HasLeftBorder + 30;

// enum KoParagraphStyle.BorderStyle
KoParagraphStyle.BorderNone = 0;
KoParagraphStyle.BorderDotted = 1;
KoParagraphStyle.BorderDashed = 2;
KoParagraphStyle.BorderSolid = 3;
KoParagraphStyle.BorderDouble = 4;
KoParagraphStyle.BorderGroove = 5;
KoParagraphStyle.BorderRidge = 6;
KoParagraphStyle.BorderInset = 7;
KoParagraphStyle.BorderOutset = 8;

// enum QTextOption.TabType
QTextOption.LeftTab = 0;
QTextOption.RightTab = QTextOption.LeftTab + 1;
QTextOption.CenterTab = QTextOption.LeftTab + 2;
QTextOption.DelimiterTab = QTextOption.LeftTab + 3;

var defaultBlockFormat = new QTextBlockFormat;
setFormatProperty(defaultBlockFormat, KoParagraphStyle.PercentLineHeight, 120);
defaultBlockFormat.clearBackground();
cursor.setBlockFormat(defaultBlockFormat);

QTextBlockFormat.clone = function(fmt) {
    var newFormat = new QTextBlockFormat;
    copyFormatProperties(newFormat, fmt);
    return newFormat;
}

// Default character formatting
var defaultFont = new QFont;
defaultFont.setPointSizeF(12.0);
defaultFont.setWeight(QFont.Normal);

var KoCharacterStyle = {};
var i = QTextFormat.UserProperty;
KoCharacterStyle.StyleId = ++i;
KoCharacterStyle.HasHyphenataion = ++i;
KoCharacterStyle.FontId = ++i;
KoCharacterStyle.StrikeOutStyle = ++i;
KoCharacterStyle.StrikeOutType = ++i;
KoCharacterStyle.StrikeOutColor = ++i;
KoCharacterStyle.StrikeOutWidth = ++i;
KoCharacterStyle.StrikeOutWeight = ++i;
KoCharacterStyle.StrikeOutMode = ++i;
KoCharacterStyle.StrikeOutText = ++i;
KoCharacterStyle.UnderlineStyle = ++i;
KoCharacterStyle.UnderlineType = ++i;
KoCharacterStyle.UnderlineWidth = ++i;
KoCharacterStyle.UnderlineWeight = ++i;
KoCharacterStyle.UnderlineMode = ++i;
KoCharacterStyle.TransformText = ++i;
KoCharacterStyle.Spelling = ++i;
KoCharacterStyle.Language = ++i;
KoCharacterStyle.Country = ++i;
KoCharacterStyle.FontCharset = ++i;

KoCharacterStyle.NoLineType = 0;
KoCharacterStyle.SingleLine = 1;
KoCharacterStyle.DoubleLine = 2;

KoCharacterStyle.NoLineStyle = 0; // Qt.NoPen;
KoCharacterStyle.SolidLine = 1; // Qt.SolidLine;
KoCharacterStyle.DottedLine = 3; // Qt.DotLine;
KoCharacterStyle.DashLine = 2; // Qt.DashLine;
KoCharacterStyle.DashDotLine = 4; // Qt.DashDotLine;
KoCharacterStyle.DashDotDotLine = 5; // Qt.DashDotDotLine;
KoCharacterStyle.LongDashLine = KoCharacterStyle.DashDotDotLine + 1;
KoCharacterStyle.WaveLine = KoCharacterStyle.DashDotDotLine + 2;

KoCharacterStyle.NoLineMode = 0;
KoCharacterStyle.ContinuousLineMode = 1;
KoCharacterStyle.SkipWhiteSpaceLineMode = 2;

i = 0;
KoCharacterStyle.AutoLineWeight = i++;
KoCharacterStyle.NormalLineWeight = i++;
KoCharacterStyle.BoldLineWeight = i++;
KoCharacterStyle.ThinLineWeight = i++;
KoCharacterStyle.DashLineWeight = i++;
KoCharacterStyle.MediumLineWeight = i++;
KoCharacterStyle.ThickLineWeight = i++;
KoCharacterStyle.PercentLineWeight = i++;
KoCharacterStyle.LengthLineWeight = i++;

var defaultTextFormat = new QTextCharFormat;
defaultTextFormat.setFont(defaultFont);
defaultTextFormat.setVerticalAlignment(QTextCharFormat.AlignNormal);
defaultTextFormat.setForeground(new QBrush(new QColor(0, 0, 0)));
setFormatProperty(defaultTextFormat, KoCharacterStyle.StrikeOutColor, new QColor(0, 0, 0));
cursor.setCharFormat(defaultTextFormat);

QTextCharFormat.clone = function(fmt) {
    var newFormat = new QTextCharFormat;
    copyFormatProperties(newFormat, fmt);
    return newFormat;
};

// Default list formatting
QTextListFormat.clone = function(fmt) {
    var newFormat = new QTextListFormat;
    copyFormatProperties(newFormat, fmt);
    return newFormat;
};

// KOffice specific
var KoListStyle = {};
KoListStyle.ListItemSuffix = 0x1003E9;
KoListStyle.StartValue = 0x1003EA;
KoListStyle.Level = 0x1003EB;

var defaultListItemFormat = QTextCharFormat.clone(defaultTextFormat); // new QTextCharFormat;

var defaultListFormat = new QTextListFormat;
setFormatProperty(defaultListFormat, KoListStyle.StartValue, 1);
setFormatProperty(defaultListFormat, KoListStyle.Level, 1);

