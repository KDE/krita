var document = new QTextDocument;
var cursor = new QTextCursor(document);

// Default paragraph formatting
var i;
var KoParagraphStyle = {};
var QTextOption = {};

i = QTextFormat.UserProperty;
KoParagraphStyle.StyleId = ++i;
KoParagraphStyle.PercentLineHeight = ++i;
KoParagraphStyle.FixedLineHeight = ++i;
KoParagraphStyle.MinimumLineHeight = ++i;
KoParagraphStyle.LineSpacing = ++i;
KoParagraphStyle.LineSpacingFromFont = ++i;
KoParagraphStyle.AlignLastLine = ++i;
KoParagraphStyle.WidowThreshold = ++i;
KoParagraphStyle.OrphanThreshold = ++i;
KoParagraphStyle.DropCaps = ++i;
KoParagraphStyle.DropCapsLength = ++i;
KoParagraphStyle.DropCapsLines = ++i;
KoParagraphStyle.DropCapsDistance = ++i;
KoParagraphStyle.FollowDocBaseline = ++i;
KoParagraphStyle.BreakBefore = ++i;
KoParagraphStyle.BreakAfter = ++i;
KoParagraphStyle.HasLeftBorder = ++i;
KoParagraphStyle.HasTopBorder = ++i;
KoParagraphStyle.HasRightBorder = ++i;
KoParagraphStyle.HasBottomBorder = ++i;
KoParagraphStyle.BorderLineWidth = ++i;
KoParagraphStyle.SecondBorderLineWidth = ++i;
KoParagraphStyle.DistanceToSecondBorder = ++i;
KoParagraphStyle.LeftPadding = ++i;
KoParagraphStyle.TopPadding = ++i;
KoParagraphStyle.RightPadding = ++i;
KoParagraphStyle.BottomPadding = ++i;
KoParagraphStyle.LeftBorderWidth = ++i;
KoParagraphStyle.LeftInnerBorderWidth = ++i;
KoParagraphStyle.LeftBorderSpacing = ++i;
KoParagraphStyle.LeftBorderStyle = ++i;
KoParagraphStyle.LeftBorderColor = ++i;
KoParagraphStyle.TopBorderWidth = ++i;
KoParagraphStyle.TopInnerBorderWidth = ++i;
KoParagraphStyle.TopBorderSpacing = ++i;
KoParagraphStyle.TopBorderStyle = ++i;
KoParagraphStyle.TopBorderColor = ++i;
KoParagraphStyle.RightBorderWidth = ++i;
KoParagraphStyle.RightInnerBorderWidth = ++i;
KoParagraphStyle.RightBorderSpacing = ++i;
KoParagraphStyle.RightBorderStyle = ++i;
KoParagraphStyle.RightBorderColor = ++i;
KoParagraphStyle.BottomBorderWidth = ++i;
KoParagraphStyle.BottomInnerBorderWidth = ++i;
KoParagraphStyle.BottomBorderSpacing = ++i;
KoParagraphStyle.BottomBorderStyle = ++i;
KoParagraphStyle.BottomBorderColor = ++i;
KoParagraphStyle.ExplicitListValue = ++i;
KoParagraphStyle.RestartListNumbering = ++i;
KoParagraphStyle.ListLevel = ++i;
KoParagraphStyle.IsListHeader = ++i;
KoParagraphStyle.AutoTextIndent = ++i;
KoParagraphStyle.TabStopDistance = ++i;
KoParagraphStyle.TabPositions = ++i;
KoParagraphStyle.TextProgressionDirection = ++i;
KoParagraphStyle.MasterPageName = ++i;


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
i = QTextFormat.UserProperty;
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
setFormatProperty(defaultListFormat, KoListStyle.ListItemSuffix, ".");

