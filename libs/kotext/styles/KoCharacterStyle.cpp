/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoCharacterStyle.h"
#include "opendocument/KoTextLoadingContext.h"

#include "Styles_p.h"

#include <QTextBlock>
#include <QTextCursor>

#include <KoStyleStack.h>
#include <KoOasisStyles.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoUnit.h>

#include <KDebug>

class KoCharacterStyle::Private {
public:
    Private() : stylesPrivate( new StylePrivate()) {}
    ~Private() {
        delete stylesPrivate;
    }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate->add(key, value);
    }
    double propertyDouble(int key) const {
        QVariant variant = stylesPrivate->value(key);
        if(variant.isNull())
            return 0.0;
        return variant.toDouble();
    }
    int propertyInt(int key) const {
        QVariant variant = stylesPrivate->value(key);
        if(variant.isNull())
            return 0;
        return variant.toInt();
    }
    QString propertyString(int key) const {
        QVariant variant = stylesPrivate->value(key);
        if(variant.isNull())
            return QString();
        return qvariant_cast<QString>(variant);
    }
    bool propertyBoolean(int key) const {
        QVariant variant = stylesPrivate->value(key);
        if(variant.isNull())
            return false;
        return variant.toBool();
    }
    QColor propertyColor(int key) const {
        QVariant variant = stylesPrivate->value(key);
        if(variant.isNull())
            return QColor();
        return variant.value<QColor>();
    }

    QString name;
    StylePrivate *stylesPrivate;
};

KoCharacterStyle::KoCharacterStyle(QObject *parent)
    : QObject(parent), d( new Private() )
{
    setFontPointSize(12.0);
    setFontWeight(QFont::Normal);
    setVerticalAlignment(QTextCharFormat::AlignNormal);
    setForeground(Qt::black);
    setFontStrikeOutColor(Qt::black);
}

KoCharacterStyle::KoCharacterStyle(const KoCharacterStyle &style)
    : QObject(0), d( new Private() )
{
    d->stylesPrivate->copyMissing(style.d->stylesPrivate);
}

KoCharacterStyle::~KoCharacterStyle() {
    delete d;
}

QPen KoCharacterStyle::textOutline () const {
    QVariant variant = d->stylesPrivate->value(QTextFormat::TextOutline);
    if(variant.isNull()) {
        QPen pen(Qt::NoPen);
        return pen;
    }
    return qvariant_cast<QPen>(variant);
}

QColor KoCharacterStyle::underlineColor () const {
    QVariant variant = d->stylesPrivate->value(QTextFormat::TextUnderlineColor);
    if(variant.isNull()) {
        QColor color;
        return color;
    }
    return qvariant_cast<QColor>(variant);
}

QBrush KoCharacterStyle::background() const {
    QVariant variant = d->stylesPrivate->value(QTextFormat::BackgroundBrush);

    if(variant.isNull()) {
        QBrush brush;
        return brush;
    }
    return qvariant_cast<QBrush>(variant);
}

void KoCharacterStyle::clearBackground() {
    d->stylesPrivate->remove(QTextCharFormat::BackgroundBrush);
}

QBrush KoCharacterStyle::foreground() const {
    QVariant variant = d->stylesPrivate->value(QTextFormat::ForegroundBrush);
    if(variant.isNull()) {
        QBrush brush;
        return brush;
    }
    return qvariant_cast<QBrush>(variant);
}

void KoCharacterStyle::clearForeground() {
    d->stylesPrivate->remove(QTextCharFormat::ForegroundBrush);
}

void KoCharacterStyle::applyStyle(QTextCharFormat &format) const {
    // copy all relevant properties.
    static const int properties[] = {
        StyleId,
        QTextFormat::FontPointSize,
        QTextCharFormat::ForegroundBrush,
        QTextFormat::FontFamily,
        QTextFormat::FontWeight,
        QTextFormat::FontItalic,
        QTextFormat::FontOverline,
        QTextFormat::FontStrikeOut,
        QTextFormat::FontFixedPitch,
        QTextFormat::TextUnderlineStyle,
        QTextFormat::TextVerticalAlignment,
        QTextFormat::TextOutline,
        QTextFormat::BackgroundBrush,
        QTextFormat::ForegroundBrush,
        QTextFormat::TextUnderlineColor,
        KoCharacterStyle::FontStrikeOutStyle,
        KoCharacterStyle::FontStrikeOutColor,
        -1
    };

    int i=0;
    while(properties[i] != -1) {
        QVariant variant = d->stylesPrivate->value(properties[i]);
        if(!variant.isNull())
            format.setProperty(properties[i], variant);
        i++;
    }
}

void KoCharacterStyle::applyStyle(QTextBlock &block) const {
    QTextCursor cursor(block);
    QTextCharFormat cf = cursor.charFormat();
/*
    TODO make replacement of the style be a lot smarter.
    QTextBlock::Iterator fragmentIter = block.begin();
 */
    cursor.setPosition(block.position() + block.length()-1, QTextCursor::KeepAnchor);
    applyStyle(cf);
    cursor.mergeCharFormat(cf);
    cursor.setBlockCharFormat(cf);
}

void KoCharacterStyle::applyStyle(QTextCursor *selection) const {
    QTextCharFormat cf = selection->charFormat();
    applyStyle(cf);
    selection->mergeCharFormat(cf);
}

// OASIS 14.2.29
static void importOasisUnderline( const QString& type, const QString& style,
                                  QTextCharFormat::UnderlineStyle& formatstyle )
{
    formatstyle = QTextCharFormat::NoUnderline;

    //TODO needs to be supported via Qt::PenStyle/Qt::CustomDashLine
    if ( type == "single" )
        formatstyle = QTextCharFormat::SingleUnderline;
    else if ( type == "double" )
        formatstyle = QTextCharFormat::SingleUnderline;

    if ( style == "solid" )
        formatstyle = QTextCharFormat::SingleUnderline;
    else if ( style == "dotted" )
        formatstyle = QTextCharFormat::DotLine;
    else if ( style == "dash" || style == "long-dash" ) // not in kotext
        formatstyle = QTextCharFormat::DashUnderline;
    else if ( style == "dot-dash" )
        formatstyle = QTextCharFormat::DashDotLine;
    else if ( style == "dot-dot-dash" )
        formatstyle = QTextCharFormat::DashDotDotLine;
    else if ( style == "wave" )
        formatstyle = QTextCharFormat::WaveUnderline;

    // TODO bold. But this is another attribute in OASIS (text-underline-width), which makes sense.
    // We should separate them in kotext...
}

void KoCharacterStyle::setFontFamily (const QString &family) {
    d->setProperty(QTextFormat::FontFamily, family);
}
QString KoCharacterStyle::fontFamily () const {
    return d->propertyString(QTextFormat::FontFamily);
}
void KoCharacterStyle::setFontPointSize (qreal size) {
    d->setProperty(QTextFormat::FontPointSize, size);
}
double KoCharacterStyle::fontPointSize () const {
    return d->propertyDouble(QTextFormat::FontPointSize);
}
void KoCharacterStyle::setFontWeight (int weight) {
    d->setProperty(QTextFormat::FontWeight, weight);
}
int KoCharacterStyle::fontWeight () const {
    return d->propertyInt(QTextFormat::FontWeight);
}
void KoCharacterStyle::setFontItalic (bool italic) {
    d->setProperty(QTextFormat::FontItalic, italic);
}
bool KoCharacterStyle::fontItalic () const {
    return d->propertyBoolean(QTextFormat::FontItalic);
}
void KoCharacterStyle::setFontOverline (bool overline) {
    d->setProperty(QTextFormat::FontOverline, overline);
}
bool KoCharacterStyle::fontOverline () const {
    return d->propertyBoolean(QTextFormat::FontOverline);
}
void KoCharacterStyle::setUnderlineColor (const QColor &color) {
    d->setProperty(QTextFormat::TextUnderlineColor, color);
}
void KoCharacterStyle::setFontFixedPitch (bool fixedPitch) {
    d->setProperty(QTextFormat::FontFixedPitch, fixedPitch);
}
bool KoCharacterStyle::fontFixedPitch () const {
    return d->propertyBoolean(QTextFormat::FontFixedPitch);
}
void KoCharacterStyle::setUnderlineStyle (QTextCharFormat::UnderlineStyle style) {
    d->setProperty(QTextFormat::TextUnderlineStyle, style);
}
QTextCharFormat::UnderlineStyle KoCharacterStyle::underlineStyle () const {
    return static_cast<QTextCharFormat::UnderlineStyle> (d->propertyInt(QTextFormat::TextUnderlineStyle));

}
void KoCharacterStyle::setVerticalAlignment (QTextCharFormat::VerticalAlignment alignment) {
    d->setProperty(QTextFormat::TextVerticalAlignment, alignment);
}
QTextCharFormat::VerticalAlignment KoCharacterStyle::verticalAlignment () const {
    return static_cast<QTextCharFormat::VerticalAlignment> (d->propertyInt(QTextFormat::TextVerticalAlignment));
}
void KoCharacterStyle::setTextOutline (const QPen &pen) {
    d->setProperty(QTextFormat::TextOutline, pen);
}
void KoCharacterStyle::setBackground (const QBrush &brush) {
    d->setProperty(QTextFormat::BackgroundBrush, brush);
}
void KoCharacterStyle::setForeground (const QBrush &brush) {
    d->setProperty(QTextFormat::ForegroundBrush, brush);
}
QString KoCharacterStyle::name() const {
    return d->name;
}
void KoCharacterStyle::setName(const QString &name) {
    d->name = name;
}
int KoCharacterStyle::styleId() const {
    return d->propertyInt(StyleId);
}
void KoCharacterStyle::setStyleId(int id) {
    d->setProperty(StyleId, id);
}
QFont KoCharacterStyle::font() const {
    QFont font;
    if(d->stylesPrivate->contains(QTextFormat::FontFamily))
        font.setFamily(fontFamily());
    if(d->stylesPrivate->contains(QTextFormat::FontPointSize))
        font.setPointSizeF(fontPointSize());
    if(d->stylesPrivate->contains(QTextFormat::FontWeight))
        font.setWeight(fontWeight());
    if(d->stylesPrivate->contains(QTextFormat::FontItalic))
        font.setItalic(fontItalic());
    return font;
}
void KoCharacterStyle::setHasHyphenation(bool on) {
    d->setProperty(HasHyphenation, on);
}
bool KoCharacterStyle::hasHyphenation() const {
    return d->propertyBoolean(HasHyphenation);
}

void KoCharacterStyle::setFontStrikeOutStyle (Qt::PenStyle strikeOut) {
    d->setProperty(FontStrikeOutStyle, strikeOut);
}

Qt::PenStyle KoCharacterStyle::fontStrikeOutStyle () const {
    return (Qt::PenStyle) d->propertyInt(FontStrikeOutStyle);
}

void KoCharacterStyle::setFontStrikeOutColor (QColor color) {
    d->setProperty(FontStrikeOutColor, color);
}

QColor KoCharacterStyle::fontStrikeOutColor () const {
    return d->propertyColor(FontStrikeOutColor);
}

bool KoCharacterStyle::hasProperty(int key) const {
    return d->stylesPrivate->contains(key);
}

//in 1.6 this was defined in KoTextFormat::load(KoOasisContext& context)
void KoCharacterStyle::loadOasis(KoTextLoadingContext& context) {
    KoStyleStack &styleStack = context.styleStack();

    // The fo:color attribute specifies the foreground color of text.
    if ( styleStack.hasProperty( KoXmlNS::fo, "color" ) ) { // 3.10.3
        if (styleStack.property( KoXmlNS::style, "use-window-font-color") != "true") {
            QColor color(styleStack.property( KoXmlNS::fo, "color" )); // #rrggbb format
            if ( color.isValid() ) {
                QBrush brush = foreground();
                brush.setColor(color);
                setForeground(brush);
            }
        }
    }

    QString fontName;
    if ( styleStack.hasProperty( KoXmlNS::fo, "font-family" ) ) {
        fontName = styleStack.property( KoXmlNS::fo, "font-family" );

        // Specify whether a font has a fixed or variable width.
        // These attributes are ignored if there is no corresponding fo:font-family attribute attached to the same formatting properties element.
        if ( styleStack.hasProperty( KoXmlNS::style, "font-pitch" ) ) {
            if ( styleStack.property( KoXmlNS::style, "font-pitch" ) == "fixed" )
                setFontFixedPitch( true );
        }
    }
    if ( styleStack.hasProperty( KoXmlNS::style, "font-family" ) )
        fontName = styleStack.property( KoXmlNS::style, "font-family" );
    if ( styleStack.hasProperty( KoXmlNS::style, "font-name" ) ) {
        // This font name is a reference to a font face declaration.
        KoOasisStyles &styles = context.oasisStyles();
        const KoXmlElement *fontFace = styles.findStyle(styleStack.property( KoXmlNS::style, "font-name" ));
        if (fontFace != 0)
            fontName = fontFace->attributeNS(KoXmlNS::svg, "font-family", "");
    }

    if ( ! fontName.isNull() ) {
        // Hmm, the remove "'" could break it's in the middle of the fontname...
        fontName = fontName.remove( "'" );

        // 'Thorndale' is not known outside OpenOffice so we substitute it
        // with 'Times New Roman' that looks nearly the same.
        if ( fontName == "Thorndale" )
            fontName = "Times New Roman";

        fontName.remove(QRegExp("\\sCE$")); // Arial CE -> Arial
        setFontFamily( fontName );
    }

    // Specify the size of a font. The value of these attribute is either an absolute length or a percentage
    if ( styleStack.hasProperty( KoXmlNS::fo, "font-size" ) ) {
        double pointSize = styleStack.fontSize( fontPointSize() );
        if (pointSize > 0)
            setFontPointSize(pointSize);
    }

    // These attributes specify a relative font size change as a length such as +1pt, -3pt. It changes the font size based on the font size of the parent style.
    if ( styleStack.hasProperty( KoXmlNS::style, "font-size-rel" ) ) {
        double pointSize = fontPointSize() + KoUnit::parseValue( styleStack.property( KoXmlNS::style, "font-size-rel" ) );
        if (pointSize > 0)
            setFontPointSize(pointSize);
    }

    // Specify the weight of a font. The permitted values are normal, bold, and numeric values 100-900, in steps of 100. Unsupported numerical values are rounded off to the next supported value.
    if ( styleStack.hasProperty( KoXmlNS::fo, "font-weight" ) ) { // 3.10.24
        QString fontWeight = styleStack.property( KoXmlNS::fo, "font-weight" );
        int boldness;
        if ( fontWeight == "normal" )
            boldness = 50;
        else if ( fontWeight == "bold" )
            boldness = 75;
        else
            // XSL/CSS has 100,200,300...900. Not the same scale as Qt!
            // See http://www.w3.org/TR/2001/REC-xsl-20011015/slice7.html#font-weight
            boldness = fontWeight.toInt() / 10;
        setFontWeight( boldness );
    }

    // Specify whether to use normal or italic font face.
    if ( styleStack.hasProperty( KoXmlNS::fo, "font-style" ) ) { // 3.10.19
        if ( styleStack.property( KoXmlNS::fo, "font-style" ) == "italic" ||
             styleStack.property( KoXmlNS::fo, "font-style" ) == "oblique" ) { // no difference in kotext
            setFontItalic( true );
        }
    }

//TODO
#if 0
    d->m_bWordByWord = styleStack.property( KoXmlNS::style, "text-underline-mode" ) == "skip-white-space";
    // TODO style:text-line-through-mode

    /*
    // OO compat code, to move to OO import filter
    d->m_bWordByWord = (styleStack.hasProperty( KoXmlNS::fo, "score-spaces")) // 3.10.25
                      && (styleStack.property( KoXmlNS::fo, "score-spaces") == "false");
    if( styleStack.hasProperty( KoXmlNS::style, "text-crossing-out" )) { // 3.10.6
        QString strikeOutType = styleStack.property( KoXmlNS::style, "text-crossing-out" );
        if( strikeOutType =="double-line")
            m_strikeOutType = S_DOUBLE;
        else if( strikeOutType =="single-line")
            m_strikeOutType = S_SIMPLE;
        else if( strikeOutType =="thick-line")
            m_strikeOutType = S_SIMPLE_BOLD;
        // not supported by KWord: "slash" and "X"
        // not supported by OO: stylelines (solid, dash, dot, dashdot, dashdotdot)
    }
    */
#endif

    // Specifies whether text is underlined, and if so, whether a single or double line will be used for underlining.
    if ( styleStack.hasProperty( KoXmlNS::style, "text-underline-type" )
        || styleStack.hasProperty( KoXmlNS::style, "text-underline-style" ) ) { // OASIS 14.4.28
        QTextCharFormat::UnderlineStyle underlineStyle;
        importOasisUnderline( styleStack.property( KoXmlNS::style, "text-underline-type" ),
                              styleStack.property( KoXmlNS::style, "text-underline-style" ),
                              underlineStyle );
        setUnderlineStyle(underlineStyle);
    }

    // Specifies the color that is used to underline text. The value of this attribute is either font-color or a color. If the value is font-color, the current text color is used for underlining.
    QString underLineColor = styleStack.property( KoXmlNS::style, "text-underline-color" ); // OO 3.10.23, OASIS 14.4.31
    if ( !underLineColor.isEmpty() && underLineColor != "font-color" )
        setUnderlineColor( QColor(underLineColor) );
    
    
    if (( styleStack.hasProperty( KoXmlNS::style, "text-line-through-type" ) ) ||  ( styleStack.hasProperty( KoXmlNS::style, "text-line-through-style" ))) { // OASIS 14.4.7
        QTextCharFormat::UnderlineStyle underlineStyle;
        importOasisUnderline( styleStack.property( KoXmlNS::style, "text-line-through-type" ),
                              styleStack.property( KoXmlNS::style, "text-line-through-style" ),
                                      underlineStyle );
        setFontStrikeOutStyle((Qt::PenStyle) underlineStyle);
    }
    
    QString lineThroughColor = styleStack.property( KoXmlNS::style, "text-line-through-color" ); // OO 3.10.23, OASIS 14.4.31
    if ( !lineThroughColor.isEmpty() && lineThroughColor != "font-color" )
        setFontStrikeOutColor( QColor(lineThroughColor) );
//TODO
#if 0
    if ( styleStack.hasProperty( KoXmlNS::style, "text-line-through-type" ) ) { // OASIS 14.4.7
        // Reuse code for loading underlines, and convert to strikeout enum (if not wave)
        UnderlineType uType; UnderlineStyle uStyle;
        importOasisUnderline( styleStack.property( KoXmlNS::style, "text-line-through-type" ),
                              styleStack.property( KoXmlNS::style, "text-line-through-style" ),
                              uType, uStyle );
        m_strikeOutType = S_NONE;
        if ( uType != U_WAVE )
            m_strikeOutType = (StrikeOutType)uType;
        m_strikeOutStyle = (StrikeOutStyle)uStyle;
    }

    // Text position
    va = AlignNormal;
    d->m_relativeTextSize = 0.58;
    d->m_offsetFromBaseLine = 0;
    if( styleStack.hasProperty( KoXmlNS::style, "text-position")) { // OO 3.10.7
        importTextPosition( styleStack.property( KoXmlNS::style, "text-position"), fn.pointSizeFloat(),
                            va, d->m_relativeTextSize, d->m_offsetFromBaseLine, context );
    }

    // Small caps, lowercase, uppercase
    m_attributeFont = ATT_NONE;
    if ( styleStack.hasProperty( KoXmlNS::fo, "font-variant" ) // 3.10.1
         || styleStack.hasProperty( KoXmlNS::fo, "text-transform" ) ) { // 3.10.2
        bool smallCaps = styleStack.property( KoXmlNS::fo, "font-variant" ) == "small-caps";
        if ( smallCaps ) {
            m_attributeFont = ATT_SMALL_CAPS;
        } else {
            QString textTransform = styleStack.property( KoXmlNS::fo, "text-transform" );
            if ( textTransform == "uppercase" )
                m_attributeFont = ATT_UPPER;
            else if ( textTransform == "lowercase" )
                m_attributeFont = ATT_LOWER;
            // TODO in KWord: "capitalize".
        }
    }

    if ( styleStack.hasProperty( KoXmlNS::fo, "language") ) { // 3.10.17
        m_language = styleStack.property( KoXmlNS::fo, "language");
        const QString country = styleStack.property( KoXmlNS::fo, "country" );
        if ( !country.isEmpty() ) {
            m_language += '_';
            m_language += country;
        }
    }
#endif

    // The fo:background-color attribute specifies the background color of a paragraph.
    if ( styleStack.hasProperty( KoXmlNS::fo, "background-color") ) {
        const QString bgcolor = styleStack.property( KoXmlNS::fo, "background-color");
        QBrush brush = background();
        if (bgcolor == "transparent")
            brush.setStyle(Qt::NoBrush);
        else {
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(bgcolor); // #rrggbb format
        }
        setBackground(brush);
    }

    // The style:use-window-font-color attribute specifies whether or not the window foreground color should be as used as the foreground color for a light background color and white for a dark background color.
    if ( styleStack.hasProperty( KoXmlNS::style, "use-window-font-color" ) ) {
        if (styleStack.property( KoXmlNS::style, "use-window-font-color") == "true") {
            // Do like OpenOffice.org : change the foreground font if its color is too close to the background color...
            QColor back = background().color();
            QColor front = foreground().color();
            if ((abs(qGray(back.rgb()) - qGray(front.rgb())) < 10) && (background().style() != Qt::NoBrush) && (foreground().style() != Qt::NoBrush)) {
                front.setRed(255 - front.red());
                front.setGreen(255 - front.green());
                front.setBlue(255 - front.blue());
                QBrush frontBrush = foreground();
                frontBrush.setColor(front);
                setForeground(frontBrush);
            }
        }
    }

//TODO
#if 0
    if ( styleStack.hasProperty( KoXmlNS::fo, "text-shadow") ) { // 3.10.21
        parseShadowFromCss( styleStack.property( KoXmlNS::fo, "text-shadow") );
    }

    d->m_bHyphenation = true;
    if ( styleStack.hasProperty( KoXmlNS::fo, "hyphenate" ) ) // it's a character property in OASIS (but not in OO-1.1)
        d->m_bHyphenation = styleStack.property( KoXmlNS::fo, "hyphenate" ) == "true";

    /*
      Missing properties:
      style:text-outline, 3.10.5 - not implemented in kotext
      style:font-family-generic, 3.10.10 - roman, swiss, modern -> map to a font?
      style:font-style-name, 3.10.11 - can be ignored, says DV, the other ways to specify a font are more precise
      style:font-charset, 3.10.14 - not necessary with Qt
      fo:letter-spacing, 3.10.16 - not implemented in kotext
      style:text-relief, 3.10.20 - not implemented in kotext
      style:letter-kerning, 3.10.20 - not implemented in kotext
      style:text-blinking, 3.10.27 - not implemented in kotext IIRC
      style:text-combine, 3.10.29/30 - not implemented, see http://www.w3.org/TR/WD-i18n-format/
      style:text-emphasis, 3.10.31 - not implemented in kotext
      style:text-scale, 3.10.33 - not implemented in kotext
      style:text-rotation-angle, 3.10.34 - not implemented in kotext (kpr rotates whole objects)
      style:text-rotation-scale, 3.10.35 - not implemented in kotext (kpr rotates whole objects)
      style:punctuation-wrap, 3.10.36 - not implemented in kotext
    */

    d->m_underLineWidth = 1.0;

    generateKey();
    addRef();
#endif

}

#include "KoCharacterStyle.moc"
