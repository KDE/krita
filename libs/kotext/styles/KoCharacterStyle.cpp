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
#include "opendocument/KoOpenDocumentLoadingContext.h"

#include "Styles_p.h"

#include <QTextBlock>
#include <QTextCursor>

#include <KoStyleStack.h>
#include <KoOasisStyles.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>

#include <KDebug>

class KoCharacterStyle::Private {
public:
    Private() : stylesPrivate( new StylePrivate()) {}

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
}

KoCharacterStyle::KoCharacterStyle(const KoCharacterStyle &style)
    : QObject(0), d( new Private() )
{
    d->stylesPrivate->copyMissing(style.d->stylesPrivate);
}

KoCharacterStyle::~KoCharacterStyle() {
}

void KoCharacterStyle::setProperty(int key, const QVariant &value) {
    d->stylesPrivate->add(key, value);
}

QVariant KoCharacterStyle::property(int key) const {
    return d->stylesPrivate->value(key);
}

double KoCharacterStyle::propertyDouble(int key) const {
    QVariant variant = d->stylesPrivate->value(key);
    if(variant.isNull())
        return 0.0;
    return variant.toDouble();
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

int KoCharacterStyle::propertyInt(int key) const {
    QVariant variant = d->stylesPrivate->value(key);
    if(variant.isNull())
        return 0;
    return variant.toInt();
}

bool KoCharacterStyle::propertyBoolean(int key) const {
    QVariant variant = d->stylesPrivate->value(key);
    if(variant.isNull())
        return false;
    return variant.toBool();
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

QString KoCharacterStyle::propertyString(int key) const {
    QVariant variant = d->stylesPrivate->value(key);
    if(variant.isNull())
        return QString();
    return qvariant_cast<QString>(variant);
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
    setProperty(QTextFormat::FontFamily, family);
}
QString KoCharacterStyle::fontFamily () const {
    return propertyString(QTextFormat::FontFamily);
}
void KoCharacterStyle::setFontPointSize (qreal size) {
    setProperty(QTextFormat::FontPointSize, size);
}
double KoCharacterStyle::fontPointSize () const {
    return propertyDouble(QTextFormat::FontPointSize);
}
void KoCharacterStyle::setFontWeight (int weight) {
    setProperty(QTextFormat::FontWeight, weight);
}
int KoCharacterStyle::fontWeight () const {
    return propertyInt(QTextFormat::FontWeight);
}
void KoCharacterStyle::setFontItalic (bool italic) {
    setProperty(QTextFormat::FontItalic, italic);
}
bool KoCharacterStyle::fontItalic () const {
    return propertyBoolean(QTextFormat::FontItalic);
}
void KoCharacterStyle::setFontOverline (bool overline) {
    setProperty(QTextFormat::FontOverline, overline);
}
bool KoCharacterStyle::fontOverline () const {
    return propertyBoolean(QTextFormat::FontOverline);
}
void KoCharacterStyle::setFontStrikeOut (bool strikeOut) {
    setProperty(QTextFormat::FontStrikeOut, strikeOut);
}
bool KoCharacterStyle::fontStrikeOut () const {
    return propertyBoolean(QTextFormat::FontStrikeOut);
}
void KoCharacterStyle::setUnderlineColor (const QColor &color) {
    setProperty(QTextFormat::TextUnderlineColor, color);
}
void KoCharacterStyle::setFontFixedPitch (bool fixedPitch) {
    setProperty(QTextFormat::FontFixedPitch, fixedPitch);
}
bool KoCharacterStyle::fontFixedPitch () const {
    return propertyBoolean(QTextFormat::FontFixedPitch);
}
void KoCharacterStyle::setUnderlineStyle (QTextCharFormat::UnderlineStyle style) {
    setProperty(QTextFormat::TextUnderlineStyle, style);
}
QTextCharFormat::UnderlineStyle KoCharacterStyle::underlineStyle () const {
    return static_cast<QTextCharFormat::UnderlineStyle> (propertyInt(QTextFormat::TextUnderlineStyle));

}
void KoCharacterStyle::setVerticalAlignment (QTextCharFormat::VerticalAlignment alignment) {
    setProperty(QTextFormat::TextVerticalAlignment, alignment);
}
QTextCharFormat::VerticalAlignment KoCharacterStyle::verticalAlignment () const {
    return static_cast<QTextCharFormat::VerticalAlignment> (propertyInt(QTextFormat::TextVerticalAlignment));
}
void KoCharacterStyle::setTextOutline (const QPen &pen) {
    setProperty(QTextFormat::TextOutline, pen);
}
void KoCharacterStyle::setBackground (const QBrush &brush) {
    setProperty(QTextFormat::BackgroundBrush, brush);
}
void KoCharacterStyle::setForeground (const QBrush &brush) {
    setProperty(QTextFormat::ForegroundBrush, brush);
}
QString KoCharacterStyle::name() const {
    return d->name;
}
void KoCharacterStyle::setName(const QString &name) {
    d->name = name;
}
int KoCharacterStyle::styleId() const {
    return propertyInt(StyleId);
}
void KoCharacterStyle::setStyleId(int id) {
    setProperty(StyleId, id);
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

//in 1.6 this was defined in KoTextFormat::load(KoOasisContext& context)
void KoCharacterStyle::loadOasis(KoOpenDocumentLoadingContext& context) {
    KoStyleStack &styleStack = context.styleStack();

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
    if ( styleStack.hasProperty( KoXmlNS::fo, "font-family" ) )
        fontName = styleStack.property( KoXmlNS::fo, "font-family" );
    if ( styleStack.hasProperty( KoXmlNS::style, "font-family" ) )
        fontName = styleStack.property( KoXmlNS::style, "font-family" );
    if ( styleStack.hasProperty( KoXmlNS::style, "font-name" ) ) {
        // This font name is a reference to a font face declaration.
        KoOasisStyles &styles = context.oasisStyles();
        const KoXmlElement *fontFace = styles.findStyle(styleStack.property( KoXmlNS::style, "font-name" ));
        if (fontFace != 0) {
            fontName = fontFace->attributeNS(KoXmlNS::svg, "font-family", "");
        }
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

    if ( styleStack.hasProperty( KoXmlNS::fo, "font-size" ) ) { // 3.10.14
        double pointSize = styleStack.fontSize();
        //fn.setPointSizeFloat( pointSize );
        if (pointSize > 0)
            setFontPointSize(pointSize);
    }

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

    if ( styleStack.hasProperty( KoXmlNS::style, "text-underline-type" )
        || styleStack.hasProperty( KoXmlNS::style, "text-underline-style" ) ) { // OASIS 14.4.28
        QTextCharFormat::UnderlineStyle underlineStyle;
        importOasisUnderline( styleStack.property( KoXmlNS::style, "text-underline-type" ),
                              styleStack.property( KoXmlNS::style, "text-underline-style" ),
                              underlineStyle );
        setUnderlineStyle(underlineStyle);
    }
#if 0
    else if ( styleStack.hasProperty( KoXmlNS::style, "text-underline" ) ) { // OO compat (3.10.22), to be moved out
        importUnderline( styleStack.property( KoXmlNS::style, "text-underline" ),
                         m_underlineType, m_underlineStyle );
    }
#endif

    QString underLineColor = styleStack.property( KoXmlNS::style, "text-underline-color" ); // OO 3.10.23, OASIS 14.4.31
    if ( !underLineColor.isEmpty() && underLineColor != "font-color" )
        setUnderlineColor( QColor(underLineColor) );

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

    if ( styleStack.hasProperty( KoXmlNS::fo, "background-color") ) {
        QColor textBackColor( styleStack.property( KoXmlNS::fo, "background-color") ); // #rrggbb format
        if (textBackColor.isValid()) {
            QBrush brush = background();
            if (brush.style() == Qt::NoBrush)
                brush.setStyle(Qt::SolidPattern);
            brush.setColor(textBackColor);
            setBackground(brush);
        }
    }

    if ( styleStack.hasProperty( KoXmlNS::style, "use-window-font-color" ) ) { // 3.10.4
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
      style:font-pitch, 3.10.12 - fixed or variable -> map to a font?
      style:font-charset, 3.10.14 - not necessary with Qt
      style:font-size-rel, 3.10.15 - TODO in StyleStack::fontSize()
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
