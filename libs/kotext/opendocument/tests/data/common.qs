var document = new QTextDocument;
var cursor = new QTextCursor(document);

// See KoCharacterStyle.cpp
var defaultFont = new QFont;
defaultFont.setPointSizeF(12.0);
defaultFont.setWeight(QFont.Normal);

var defaultTextFormat = new QTextCharFormat;
defaultTextFormat.setFont(defaultFont);
defaultTextFormat.setVerticalAlignment(QTextCharFormat.AlignNormal);

QTextCharFormat.clone = function(fmt) {
    var newFormat = new QTextCharFormat;
    // FIXME: What we really need to do is -> newFormat.properties = fmt.properties;

    newFormat.setFont(fmt.font());
    // CHECKME: When new formats are created, the color is explicitly set to Black
    if (newFormat.foreground().style() == 0) {
        newFormat.setForeground(new QBrush(new QColor(0, 0, 0)));
    }
    return newFormat;
};

