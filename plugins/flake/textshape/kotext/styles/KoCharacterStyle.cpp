/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
 *  Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
 * Copyright (C) 2011 Stuart Dickson <stuart@furkinfantasic.net>
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

#include "Styles_p.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QFontDatabase>

#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoUnit.h>
#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoGenStyle.h>
#include <KoShadowStyle.h>
#include <KoShapeLoadingContext.h>
#include <KoStyleStack.h>
#include "KoTextDocument.h"

#ifdef SHOULD_BUILD_FONT_CONVERSION
#include <string.h>
#include <fontconfig/fontconfig.h>
#include <fontconfig/fcfreetype.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_TYPES_H
#include FT_OUTLINE_H
#include FT_RENDER_H
#include FT_TRUETYPE_TABLES_H
#include FT_SFNT_NAMES_H
#endif

#include "TextDebug.h"
#include "KoTextDebug.h"

#ifdef SHOULD_BUILD_FONT_CONVERSION
    QMap<QString,qreal> textScaleMap;
#endif //SHOULD_BUILD_FONT_CONVERSION

class Q_DECL_HIDDEN KoCharacterStyle::Private
{
public:
    Private();
    ~Private() { }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }
    qreal propertyDouble(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull()) {
            if (parentStyle)
                return parentStyle->d->propertyDouble(key);
            else if (defaultStyle)
                return defaultStyle->d->propertyDouble(key);
            return 0.0;
        }
        return variant.toDouble();
    }
    int propertyInt(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull()) {
            if (parentStyle)
                return parentStyle->d->propertyInt(key);
            else if (defaultStyle)
                return defaultStyle->d->propertyInt(key);
            return 0;
        }
        return variant.toInt();
    }
    QString propertyString(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull()) {
            if (parentStyle)
                return parentStyle->d->propertyString(key);
            else if (defaultStyle)
                return defaultStyle->d->propertyString(key);
            return QString();
        }
        return qvariant_cast<QString>(variant);
    }
    bool propertyBoolean(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull()) {
            if (parentStyle)
                return parentStyle->d->propertyBoolean(key);
            else if (defaultStyle)
                return defaultStyle->d->propertyBoolean(key);
            return false;
        }
        return variant.toBool();
    }
    QColor propertyColor(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull()) {
            if (parentStyle)
                return parentStyle->d->propertyColor(key);
            else if (defaultStyle)
                return defaultStyle->d->propertyColor(key);
            return QColor();
        }
        return variant.value<QColor>();
    }

    // problem with fonts in linux and windows is that true type fonts have more than one metric
    // they have normal metric placed in font header table
    //           microsoft metric placed in os2 table
    //           apple metric placed in os2 table
    // ms-word is probably using CreateFontIndirect and GetOutlineTextMetric function to calculate line height
    // and this functions are using windows gdi environment which is using microsoft font metric placed in os2 table
    // qt on linux is using normal font metric
    // this two metrics are different and change from font to font
    // this font stretch is needed if we want to have exact line height as in ms-word and oo
    //
    // font_size * font_stretch = windows_font_height
    qreal calculateFontYStretch(const QString &fontFamily);


    StylePrivate hardCodedDefaultStyle;

    QString name;
    StylePrivate stylesPrivate;
    KoCharacterStyle *parentStyle;
    KoCharacterStyle *defaultStyle;
    bool m_inUse;
};

KoCharacterStyle::Private::Private()
    : parentStyle(0), defaultStyle(0), m_inUse(false)
{
    //set the minimal default properties
    hardCodedDefaultStyle.add(QTextFormat::FontFamily, QString("Sans Serif"));
    hardCodedDefaultStyle.add(QTextFormat::FontPointSize, 12.0);
    hardCodedDefaultStyle.add(QTextFormat::ForegroundBrush, QBrush(Qt::black));
    hardCodedDefaultStyle.add(KoCharacterStyle::FontYStretch, 1);
    hardCodedDefaultStyle.add(QTextFormat::FontHintingPreference, QFont::PreferNoHinting);
}


void KoCharacterStyle::ensureMinimalProperties(QTextCharFormat &format) const
{
    if (d->defaultStyle) {
        QMap<int, QVariant> props = d->defaultStyle->d->stylesPrivate.properties();
        QMap<int, QVariant>::const_iterator it = props.constBegin();
        while (it != props.constEnd()) {
            // in case there is already a foreground color don't apply the use window font color as then the foreground color
            // should be used.
            if (it.key() == KoCharacterStyle::UseWindowFontColor && format.hasProperty(QTextFormat::ForegroundBrush)) {
                ++it;
                continue;
            }
            // in case there is already a use window font color don't apply the foreground brush as this overwrite the foreground color
            if (it.key() == QTextFormat::ForegroundBrush && format.hasProperty(KoCharacterStyle::UseWindowFontColor)) {
                ++it;
                continue;
            }

            if (!it.value().isNull() && !format.hasProperty(it.key())) {
                format.setProperty(it.key(), it.value());
            }
            ++it;
        }
    }
    QMap<int, QVariant> props = d->hardCodedDefaultStyle.properties();
    QMap<int, QVariant>::const_iterator it = props.constBegin();
    while (it != props.constEnd()) {
        if (!it.value().isNull() && !format.hasProperty(it.key())) {
            if (it.key() == QTextFormat::ForegroundBrush && format.hasProperty(KoCharacterStyle::UseWindowFontColor)) {
                ++it;
                continue;
            }

            format.setProperty(it.key(), it.value());
        }
        ++it;
    }
}

qreal KoCharacterStyle::Private::calculateFontYStretch(const QString &fontFamily)
{
    qreal stretch = 1;
#ifdef SHOULD_BUILD_FONT_CONVERSION

    if (textScaleMap.contains(fontFamily)) {
        return textScaleMap.value(fontFamily);
    }

    FcResult result = FcResultMatch;
    FT_Library  library;
    FT_Face face;
    int id = 0;
    int error = 0;
    QByteArray fontName = fontFamily.toLatin1();

    //TODO https://freedesktop.org/software/fontconfig/fontconfig-devel/x19.html
    // we should specify slant and weight too
    FcPattern *font = FcPatternBuild (0, FC_FAMILY, FcTypeString,fontName.data(), FC_SIZE, FcTypeDouble, (qreal)11, 0);
    if (font == 0) {
        return 1;
    }

    // find font
    FcPattern *matched = 0;
    matched = FcFontMatch (0, font, &result);
    if (matched == 0) {
        FcPatternDestroy (font);
        return 1;
    }

    // get font family name
    char * str = 0;
    result = FcPatternGetString (matched, FC_FAMILY, 0,(FcChar8**) &str);
    if (result != FcResultMatch || str == 0) {
        FcPatternDestroy (font);
        FcPatternDestroy (matched);
        return 1;
    }

    // check if right font was found
    QByteArray foundFontFamily = QByteArray::fromRawData(str, strlen(str));
    if (foundFontFamily != fontName) {
        FcPatternDestroy (font);
        FcPatternDestroy (matched);
        return 1;
    }

    // get path to font
    str = 0;
    result = FcPatternGetString (matched, FC_FILE, 0,(FcChar8**) &str);
    if (result != FcResultMatch) {
        FcPatternDestroy (font);
        FcPatternDestroy (matched);
        return 1;
    }

    // get index of font inside the font file
    result = FcPatternGetInteger (matched, FC_INDEX, 0, &id);
    if (result != FcResultMatch) {
        FcPatternDestroy (font);
        FcPatternDestroy (matched);
        return 1;
    }

    // initialize freetype
    error = FT_Init_FreeType( &library );
    if (error) {
        FcPatternDestroy (font);
        FcPatternDestroy (matched);
        return 1;
    }

    // get font metric
    error = FT_New_Face (library,(char *) str, id, &face);
    if (error) {
        FT_Done_FreeType(library);
        FcPatternDestroy (font);
        FcPatternDestroy (matched);
        return 1;
    }

    // get font metric os2 table
    TT_OS2      *os2;
    os2 = (TT_OS2 *) FT_Get_Sfnt_Table (face, ft_sfnt_os2);
    if(os2 == 0) {
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        FcPatternDestroy (font);
        FcPatternDestroy (matched);
        return 1;
    }

    // get font metric header table
    TT_Header   *header;
    header = (TT_Header *) FT_Get_Sfnt_Table (face, ft_sfnt_head);
    if(header == 0) {
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        FcPatternDestroy (font);
        FcPatternDestroy (matched);
        return 1;
    }

    // check if the data is valid
    if (header->Units_Per_EM == 0 || (os2->usWinAscent + os2->usWinDescent) == 0) {
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        FcPatternDestroy (font);
        FcPatternDestroy (matched);
        return 1;
    }

    // compute font height stretch
    // font_size * font_stretch = windows_font_height
    qreal height = os2->usWinAscent + os2->usWinDescent;
    height = height * (2048 / header->Units_Per_EM);
    stretch = (1.215 * height)/2500;
    stretch = (1.15 * height)/2500; // seems a better guess but probably not right

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    FcPatternDestroy (font);
    FcPatternDestroy (matched);

    textScaleMap.insert(fontFamily, stretch);
#else
    Q_UNUSED(fontFamily);
#endif //SHOULD_BUILD_FONT_CONVERSION

    return stretch;
}
KoCharacterStyle::KoCharacterStyle(QObject *parent)
        : QObject(parent), d(new Private())
{
}

KoCharacterStyle::KoCharacterStyle(const QTextCharFormat &format, QObject *parent)
        : QObject(parent), d(new Private())
{
    copyProperties(format);
}

KoCharacterStyle::Type KoCharacterStyle::styleType() const
{
    return KoCharacterStyle::CharacterStyle;
}

void KoCharacterStyle::copyProperties(const KoCharacterStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
    d->parentStyle = style->d->parentStyle;
    d->defaultStyle = style->d->defaultStyle;
}

void KoCharacterStyle::copyProperties(const QTextCharFormat &format)
{
    d->stylesPrivate = format.properties();
}

KoCharacterStyle *KoCharacterStyle::clone(QObject *parent) const
{
    KoCharacterStyle *newStyle = new KoCharacterStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}

KoCharacterStyle::~KoCharacterStyle()
{
    delete d;
}

void KoCharacterStyle::setDefaultStyle(KoCharacterStyle *defaultStyle)
{
    d->defaultStyle = defaultStyle;
}

void KoCharacterStyle::setParentStyle(KoCharacterStyle *parent)
{
    d->parentStyle = parent;
}

KoCharacterStyle *KoCharacterStyle::parentStyle() const
{
    return d->parentStyle;
}

QPen KoCharacterStyle::textOutline() const
{
    QVariant variant = value(QTextFormat::TextOutline);
    if (variant.isNull()) {
        return QPen(Qt::NoPen);
    }
    return qvariant_cast<QPen>(variant);
}

QBrush KoCharacterStyle::background() const
{
    QVariant variant = value(QTextFormat::BackgroundBrush);

    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KoCharacterStyle::clearBackground()
{
    d->stylesPrivate.remove(QTextCharFormat::BackgroundBrush);
}

QBrush KoCharacterStyle::foreground() const
{
    QVariant variant = value(QTextFormat::ForegroundBrush);
    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KoCharacterStyle::clearForeground()
{
    d->stylesPrivate.remove(QTextCharFormat::ForegroundBrush);
}

void KoCharacterStyle::applyStyle(QTextCharFormat &format, bool emitSignal) const
{
    if (d->parentStyle) {
        d->parentStyle->applyStyle(format);
    }

    bool fontSizeSet = false; // if this style has already set size don't apply the relatives
    const QMap<int, QVariant> props = d->stylesPrivate.properties();
    QMap<int, QVariant>::const_iterator it = props.begin();
    QList<int> clearProperty;
    while (it != props.end()) {
        if (!it.value().isNull()) {
            if (it.key() == KoCharacterStyle::PercentageFontSize && !fontSizeSet) {
                qreal size = it.value().toDouble() / 100.0;
                if (format.hasProperty(QTextFormat::FontPointSize)) {
                    size *= format.doubleProperty(QTextFormat::FontPointSize);
                } else {
                    size *= 12.0;
                }
                format.setProperty(QTextFormat::FontPointSize, size);
            }
            else if (it.key() == KoCharacterStyle::AdditionalFontSize && !fontSizeSet) {
                qreal size = it.value().toDouble() / 100.0;
                if (format.hasProperty(QTextFormat::FontPointSize)) {
                    size += format.doubleProperty(QTextFormat::FontPointSize);
                } else {
                    size += 12.0;
                }
                format.setProperty(QTextFormat::FontPointSize, size);
            }
            else if (it.key() == QTextFormat::FontFamily) {
                if (!props.contains(QTextFormat::FontStyleHint)) {
                    clearProperty.append(QTextFormat::FontStyleHint);
                }
                if (!props.contains(QTextFormat::FontFixedPitch)) {
                    clearProperty.append(QTextFormat::FontFixedPitch);
                }
                if (!props.contains(KoCharacterStyle::FontCharset)) {
                    clearProperty.append(KoCharacterStyle::FontCharset);
                }
                format.setProperty(it.key(), it.value());
            }
            else {
                debugText << "setProperty" << it.key() << it.value();
                format.setProperty(it.key(), it.value());
            }

            if (it.key() == QTextFormat::FontPointSize) {
                fontSizeSet = true;
            }

            if (it.key() == QTextFormat::ForegroundBrush) {
                clearProperty.append(KoCharacterStyle::UseWindowFontColor);
            }
            else if (it.key() == KoCharacterStyle::UseWindowFontColor) {
                clearProperty.append(QTextFormat::ForegroundBrush);
            }
        }
        ++it;
    }

    foreach (int property, clearProperty) {
        debugText << "clearProperty" << property;
        format.clearProperty(property);
    }
    if (emitSignal) {
        emit styleApplied(this);
        d->m_inUse = true;
    }
}

KoCharacterStyle *KoCharacterStyle::autoStyle(const QTextCharFormat &format, QTextCharFormat blockCharFormat) const
{
    KoCharacterStyle *autoStyle = new KoCharacterStyle(format);
    applyStyle(blockCharFormat, false);
    ensureMinimalProperties(blockCharFormat);
    autoStyle->removeDuplicates(blockCharFormat);
    autoStyle->setParentStyle(const_cast<KoCharacterStyle*>(this));
    // remove StyleId if it is there as it is not a property of the style itself and will not be written out
    // so it should not be part of the autostyle. As otherwise it can happen that the StyleId is the only
    // property left and then we write out an empty style which is unneeded.
    // we also need to remove the properties of links as they are saved differently
    autoStyle->d->stylesPrivate.remove(StyleId);
    autoStyle->d->stylesPrivate.remove(QTextFormat::IsAnchor);
    autoStyle->d->stylesPrivate.remove(QTextFormat::AnchorHref);
    autoStyle->d->stylesPrivate.remove(QTextFormat::AnchorName);
    return autoStyle;
}

struct FragmentData
{
    FragmentData(const QTextCharFormat &format, int position, int length)
    : format(format)
    , position(position)
    , length(length)
    {}

    QTextCharFormat format;
    int position;
    int length;
};

void KoCharacterStyle::applyStyle(QTextBlock &block) const
{
    QTextCursor cursor(block);
    QTextCharFormat cf = block.charFormat();

    if (!cf.isTableCellFormat()) {
        cf = KoTextDocument(block.document()).frameCharFormat();
    }

    applyStyle(cf);
    ensureMinimalProperties(cf);
    cursor.setBlockCharFormat(cf);

    // be sure that we keep the InlineInstanceId, anchor information and ChangeTrackerId when applying a style

    QList<FragmentData> fragments;
    for (QTextBlock::iterator it = block.begin(); it != block.end(); ++it) {
        QTextFragment currentFragment = it.fragment();
        if (currentFragment.isValid()) {
            QTextCharFormat format(cf);
            QVariant v = currentFragment.charFormat().property(InlineInstanceId);
            if (!v.isNull()) {
                format.setProperty(InlineInstanceId, v);
            }

            v = currentFragment.charFormat().property(ChangeTrackerId);
            if (!v.isNull()) {
                format.setProperty(ChangeTrackerId, v);
            }

            if (currentFragment.charFormat().isAnchor()) {
                format.setAnchor(true);
                format.setAnchorHref(currentFragment.charFormat().anchorHref());
            }
            fragments.append(FragmentData(format, currentFragment.position(), currentFragment.length()));
        }
    }

    foreach (const FragmentData &fragment, fragments) {
        cursor.setPosition(fragment.position);
        cursor.setPosition(fragment.position + fragment.length, QTextCursor::KeepAnchor);
        cursor.setCharFormat(fragment.format);
    }
}

void KoCharacterStyle::applyStyle(QTextCursor *selection) const
{
// FIXME below should be done for each frament in the selection
    QTextCharFormat cf = selection->charFormat();
    applyStyle(cf);
    ensureMinimalProperties(cf);
    selection->setCharFormat(cf);
}

void KoCharacterStyle::unapplyStyle(QTextCharFormat &format) const
{
    if (d->parentStyle)
        d->parentStyle->unapplyStyle(format);

    QMap<int, QVariant> props = d->stylesPrivate.properties();
    QMap<int, QVariant>::const_iterator it = props.constBegin();
    while (it != props.constEnd()) {
        if (!it.value().isNull() && it.value() == format.property(it.key())) {
           format.clearProperty(it.key());
        }
        ++it;
    }

    props = d->hardCodedDefaultStyle.properties();
    it = props.constBegin();
    while (it != props.constEnd()) {
        if (!it.value().isNull() && !format.hasProperty(it.key())) {
            format.setProperty(it.key(), it.value());
        }
        ++it;
    }
}

bool KoCharacterStyle::isApplied() const
{
    return d->m_inUse;
}

void KoCharacterStyle::unapplyStyle(QTextBlock &block) const
{
    QTextCursor cursor(block);
    QTextCharFormat cf = cursor.blockCharFormat();
    unapplyStyle(cf);
    cursor.setBlockCharFormat(cf);

    if (block.length() == 1) // only the linefeed
        return;
    QTextBlock::iterator iter = block.end();
    do {
        --iter;
        QTextFragment fragment = iter.fragment();
        cursor.setPosition(fragment.position() + 1);
        cf = cursor.charFormat();
        unapplyStyle(cf);
        cursor.setPosition(fragment.position());
        cursor.setPosition(fragment.position() + fragment.length(), QTextCursor::KeepAnchor);
        cursor.setCharFormat(cf);
    } while (iter != block.begin());
}

// OASIS 14.2.29
static void parseOdfLineWidth(const QString &width, KoCharacterStyle::LineWeight &lineWeight, qreal &lineWidth)
{
    lineWidth = 0;
    lineWeight = KoCharacterStyle::AutoLineWeight;
    if (width.isEmpty() || width == "auto")
        lineWeight = KoCharacterStyle::AutoLineWeight;
    else if (width == "normal")
        lineWeight = KoCharacterStyle::NormalLineWeight;
    else if (width == "bold")
        lineWeight = KoCharacterStyle::BoldLineWeight;
    else if (width == "thin")
        lineWeight = KoCharacterStyle::ThinLineWeight;
    else if (width == "dash")
        lineWeight = KoCharacterStyle::DashLineWeight;
    else if (width == "medium")
        lineWeight = KoCharacterStyle::MediumLineWeight;
    else if (width == "thick")
        lineWeight = KoCharacterStyle::ThickLineWeight;
    else if (width.endsWith('%')) {
        lineWeight = KoCharacterStyle::PercentLineWeight;
        lineWidth = width.mid(0, width.length() - 1).toDouble();
    } else if (width[width.length()-1].isNumber()) {
        lineWeight = KoCharacterStyle::LengthLineWeight;
        lineWidth = width.toDouble();
    } else {
        lineWeight = KoCharacterStyle::LengthLineWeight;
        lineWidth = KoUnit::parseValue(width);
    }
}

// OASIS 14.2.29
static void importOdfLine(const QString &type, const QString &style, KoCharacterStyle::LineStyle &lineStyle, KoCharacterStyle::LineType &lineType)
{
    lineStyle = KoCharacterStyle::NoLineStyle;
    lineType = KoCharacterStyle::NoLineType;

    QString fixedType = type;
    QString fixedStyle = style;
    if (fixedStyle == "none")
        fixedType.clear();
    else if (fixedType.isEmpty() && !fixedStyle.isEmpty())
        fixedType = "single";
    else if (!fixedType.isEmpty() && fixedType != "none" && fixedStyle.isEmpty()) {
        // don't set a style when the type is none
        fixedStyle = "solid";
    }

    if (fixedType == "single")
        lineType = KoCharacterStyle::SingleLine;
    else if (fixedType == "double")
        lineType = KoCharacterStyle::DoubleLine;

    if (fixedStyle == "solid")
        lineStyle = KoCharacterStyle::SolidLine;
    else if (fixedStyle == "dotted")
        lineStyle = KoCharacterStyle::DottedLine;
    else if (fixedStyle == "dash")
        lineStyle = KoCharacterStyle::DashLine;
    else if (fixedStyle == "long-dash")
        lineStyle = KoCharacterStyle::LongDashLine;
    else if (fixedStyle == "dot-dash")
        lineStyle = KoCharacterStyle::DotDashLine;
    else if (fixedStyle == "dot-dot-dash")
        lineStyle = KoCharacterStyle::DotDotDashLine;
    else if (fixedStyle == "wave")
        lineStyle = KoCharacterStyle::WaveLine;
}

static QString exportOdfLineType(KoCharacterStyle::LineType lineType)
{
    switch (lineType) {
    case KoCharacterStyle::NoLineType:
        return "none";
    case KoCharacterStyle::SingleLine:
        return "single";
    case KoCharacterStyle::DoubleLine:
        return "double";
    default:
        return "";
    }
}

static QString exportOdfLineStyle(KoCharacterStyle::LineStyle lineStyle)
{
    switch (lineStyle) {
    case KoCharacterStyle::NoLineStyle:
        return "none";
    case KoCharacterStyle::SolidLine:
        return "solid";
    case KoCharacterStyle::DottedLine:
        return "dotted";
    case KoCharacterStyle::DashLine:
        return "dash";
    case KoCharacterStyle::LongDashLine:
        return "long-dash";
    case KoCharacterStyle::DotDashLine:
        return "dot-dash";
    case KoCharacterStyle::DotDotDashLine:
        return "dot-dot-dash";
    case KoCharacterStyle::WaveLine:
        return "wave";
    default:
        return "";
    }
}
static QString exportOdfLineMode(KoCharacterStyle::LineMode lineMode)
{
    switch (lineMode) {
    case KoCharacterStyle::ContinuousLineMode:
        return "continuous";
    case KoCharacterStyle::SkipWhiteSpaceLineMode:
        return "skip-white-space";
    default:
        return "";
    }
}

static QString exportOdfLineWidth(KoCharacterStyle::LineWeight lineWeight, qreal lineWidth)
{
    switch (lineWeight) {
    case KoCharacterStyle::AutoLineWeight:
        return "auto";
    case KoCharacterStyle::NormalLineWeight:
        return "normal";
    case KoCharacterStyle::BoldLineWeight:
        return "bold";
    case KoCharacterStyle::ThinLineWeight:
        return "thin";
    case KoCharacterStyle::DashLineWeight:
        return "dash";
    case KoCharacterStyle::MediumLineWeight:
        return "medium";
    case KoCharacterStyle::ThickLineWeight:
        return "thick";
    case KoCharacterStyle::PercentLineWeight:
        return QString("%1%").arg(lineWidth);
    case KoCharacterStyle::LengthLineWeight:
        return QString("%1pt").arg(lineWidth);
    default:
        return QString();
    }
}

static QString exportOdfFontStyleHint(QFont::StyleHint hint)
{
    switch (hint) {
    case QFont::Serif:
        return "roman";
    case QFont::SansSerif:
        return "swiss";
    case QFont::TypeWriter:
        return "modern";
    case QFont::Decorative:
        return "decorative";
    case QFont::System:
        return "system";
        /*case QFont::Script */
    default:
        return "";
    }
}

void KoCharacterStyle::setFontFamily(const QString &family)
{
    d->setProperty(QTextFormat::FontFamily, family);
    setFontYStretch(d->calculateFontYStretch(family));
}
QString KoCharacterStyle::fontFamily() const
{
    return d->propertyString(QTextFormat::FontFamily);
}
void KoCharacterStyle::setFontPointSize(qreal size)
{
    d->setProperty(QTextFormat::FontPointSize, size);
}
void KoCharacterStyle::clearFontPointSize() {
    d->stylesPrivate.remove(QTextFormat::FontPointSize);
}
qreal KoCharacterStyle::fontPointSize() const
{
    return d->propertyDouble(QTextFormat::FontPointSize);
}
void KoCharacterStyle::setFontWeight(int weight)
{
    d->setProperty(QTextFormat::FontWeight, weight);
}
int KoCharacterStyle::fontWeight() const
{
    return d->propertyInt(QTextFormat::FontWeight);
}
void KoCharacterStyle::setFontItalic(bool italic)
{
    d->setProperty(QTextFormat::FontItalic, italic);
}
bool KoCharacterStyle::fontItalic() const
{
    return d->propertyBoolean(QTextFormat::FontItalic);
}
///TODO Review legacy fontOverline functions and testing (consider removal)
/*
void KoCharacterStyle::setFontOverline(bool overline)
{
    d->setProperty(QTextFormat::FontOverline, overline);
}
bool KoCharacterStyle::fontOverline() const
{
    return d->propertyBoolean(QTextFormat::FontOverline);
}
*/
void KoCharacterStyle::setFontFixedPitch(bool fixedPitch)
{
    d->setProperty(QTextFormat::FontFixedPitch, fixedPitch);
}
bool KoCharacterStyle::fontFixedPitch() const
{
    return d->propertyBoolean(QTextFormat::FontFixedPitch);
}
void KoCharacterStyle::setFontStyleHint(QFont::StyleHint styleHint)
{
    d->setProperty(QTextFormat::FontStyleHint, styleHint);
}
QFont::StyleHint KoCharacterStyle::fontStyleHint() const
{
    return static_cast<QFont::StyleHint>(d->propertyInt(QTextFormat::FontStyleHint));
}
void KoCharacterStyle::setFontKerning(bool enable)
{
    d->setProperty(QTextFormat::FontKerning, enable);
}
bool KoCharacterStyle::fontKerning() const
{
    return d->propertyBoolean(QTextFormat::FontKerning);
}
void KoCharacterStyle::setVerticalAlignment(QTextCharFormat::VerticalAlignment alignment)
{
    d->setProperty(QTextFormat::TextVerticalAlignment, alignment);
}
QTextCharFormat::VerticalAlignment KoCharacterStyle::verticalAlignment() const
{
    return static_cast<QTextCharFormat::VerticalAlignment>(d->propertyInt(QTextFormat::TextVerticalAlignment));
}
void KoCharacterStyle::setTextOutline(const QPen &pen)
{
    d->setProperty(QTextFormat::TextOutline, pen);
}
void KoCharacterStyle::setBackground(const QBrush &brush)
{
    d->setProperty(QTextFormat::BackgroundBrush, brush);
}
void KoCharacterStyle::setForeground(const QBrush &brush)
{
    d->setProperty(QTextFormat::ForegroundBrush, brush);
}
void KoCharacterStyle::setFontAutoColor(bool use)
{
    d->setProperty(KoCharacterStyle::UseWindowFontColor, use);
}

QString KoCharacterStyle::name() const
{
    return d->name;
}
void KoCharacterStyle::setName(const QString &name)
{
    if (name == d->name)
        return;
    d->name = name;
    emit nameChanged(name);
}
int KoCharacterStyle::styleId() const
{
    return d->propertyInt(StyleId);
}
void KoCharacterStyle::setStyleId(int id)
{
    d->setProperty(StyleId, id);
}
QFont KoCharacterStyle::font() const
{
    QFont font;
    if (d->stylesPrivate.contains(QTextFormat::FontFamily))
        font.setFamily(fontFamily());
    if (d->stylesPrivate.contains(QTextFormat::FontPointSize))
        font.setPointSizeF(fontPointSize());
    if (d->stylesPrivate.contains(QTextFormat::FontWeight))
        font.setWeight(fontWeight());
    if (d->stylesPrivate.contains(QTextFormat::FontItalic))
        font.setItalic(fontItalic());
    return font;
}
void KoCharacterStyle::setHasHyphenation(bool on)
{
    d->setProperty(HasHyphenation, on);
}
bool KoCharacterStyle::hasHyphenation() const
{
    return d->propertyBoolean(HasHyphenation);
}

void KoCharacterStyle::setHyphenationPushCharCount(int count)
{
    if (count > 0)
        d->setProperty(HyphenationPushCharCount, count);
    else
        d->stylesPrivate.remove(HyphenationPushCharCount);
}

int KoCharacterStyle::hyphenationPushCharCount() const
{
    if (hasProperty(HyphenationPushCharCount))
        return d->propertyInt(HyphenationPushCharCount);
    return 0;
}

void KoCharacterStyle::setHyphenationRemainCharCount(int count)
{
    if (count > 0)
        d->setProperty(HyphenationRemainCharCount, count);
    else
        d->stylesPrivate.remove(HyphenationRemainCharCount);
}

int KoCharacterStyle::hyphenationRemainCharCount() const
{
    if (hasProperty(HyphenationRemainCharCount))
        return d->propertyInt(HyphenationRemainCharCount);
    return 0;
}

void KoCharacterStyle::setStrikeOutStyle(KoCharacterStyle::LineStyle strikeOut)
{
    d->setProperty(StrikeOutStyle, strikeOut);
}

KoCharacterStyle::LineStyle KoCharacterStyle::strikeOutStyle() const
{
    return (KoCharacterStyle::LineStyle) d->propertyInt(StrikeOutStyle);
}

void KoCharacterStyle::setStrikeOutType(LineType lineType)
{
    d->setProperty(StrikeOutType, lineType);
}

KoCharacterStyle::LineType KoCharacterStyle::strikeOutType() const
{
    return (KoCharacterStyle::LineType) d->propertyInt(StrikeOutType);
}

void KoCharacterStyle::setStrikeOutColor(const QColor &color)
{
    d->setProperty(StrikeOutColor, color);
}

QColor KoCharacterStyle::strikeOutColor() const
{
    return d->propertyColor(StrikeOutColor);
}

void KoCharacterStyle::setStrikeOutWidth(LineWeight weight, qreal width)
{
    d->setProperty(KoCharacterStyle::StrikeOutWeight, weight);
    d->setProperty(KoCharacterStyle::StrikeOutWidth, width);
}

void KoCharacterStyle::strikeOutWidth(LineWeight &weight, qreal &width) const
{
    weight = (KoCharacterStyle::LineWeight) d->propertyInt(KoCharacterStyle::StrikeOutWeight);
    width = d->propertyDouble(KoCharacterStyle::StrikeOutWidth);
}
void KoCharacterStyle::setStrikeOutMode(LineMode lineMode)
{
    d->setProperty(StrikeOutMode, lineMode);
}

void KoCharacterStyle::setStrikeOutText(const QString &text)
{
    d->setProperty(StrikeOutText, text);
}
QString KoCharacterStyle::strikeOutText() const
{
    return d->propertyString(StrikeOutText);
}
KoCharacterStyle::LineMode KoCharacterStyle::strikeOutMode() const
{
    return (KoCharacterStyle::LineMode) d->propertyInt(StrikeOutMode);
}

void KoCharacterStyle::setOverlineStyle(KoCharacterStyle::LineStyle overline)
{
    d->setProperty(OverlineStyle, overline);
}

KoCharacterStyle::LineStyle KoCharacterStyle::overlineStyle() const
{
    return (KoCharacterStyle::LineStyle) d->propertyInt(OverlineStyle);
}

void KoCharacterStyle::setOverlineType(LineType lineType)
{
    d->setProperty(OverlineType, lineType);
}

KoCharacterStyle::LineType KoCharacterStyle::overlineType() const
{
    return (KoCharacterStyle::LineType) d->propertyInt(OverlineType);
}

void KoCharacterStyle::setOverlineColor(const QColor &color)
{
    d->setProperty(KoCharacterStyle::OverlineColor, color);
}

QColor KoCharacterStyle::overlineColor() const
{
    return d->propertyColor(KoCharacterStyle::OverlineColor);
}

void KoCharacterStyle::setOverlineWidth(LineWeight weight, qreal width)
{
    d->setProperty(KoCharacterStyle::OverlineWeight, weight);
    d->setProperty(KoCharacterStyle::OverlineWidth, width);
}

void KoCharacterStyle::overlineWidth(LineWeight &weight, qreal &width) const
{
    weight = (KoCharacterStyle::LineWeight) d->propertyInt(KoCharacterStyle::OverlineWeight);
    width = d->propertyDouble(KoCharacterStyle::OverlineWidth);
}

void KoCharacterStyle::setOverlineMode(LineMode mode)
{
    d->setProperty(KoCharacterStyle::OverlineMode, mode);
}

KoCharacterStyle::LineMode KoCharacterStyle::overlineMode() const
{
    return static_cast<KoCharacterStyle::LineMode>(d->propertyInt(KoCharacterStyle::OverlineMode));
}

void KoCharacterStyle::setUnderlineStyle(KoCharacterStyle::LineStyle underline)
{
    d->setProperty(UnderlineStyle, underline);
}

KoCharacterStyle::LineStyle KoCharacterStyle::underlineStyle() const
{
    return (KoCharacterStyle::LineStyle) d->propertyInt(UnderlineStyle);
}

void KoCharacterStyle::setUnderlineType(LineType lineType)
{
    d->setProperty(UnderlineType, lineType);
}

KoCharacterStyle::LineType KoCharacterStyle::underlineType() const
{
    return (KoCharacterStyle::LineType) d->propertyInt(UnderlineType);
}

void KoCharacterStyle::setUnderlineColor(const QColor &color)
{
    d->setProperty(QTextFormat::TextUnderlineColor, color);
}

QColor KoCharacterStyle::underlineColor() const
{
    return d->propertyColor(QTextFormat::TextUnderlineColor);
}

void KoCharacterStyle::setUnderlineWidth(LineWeight weight, qreal width)
{
    d->setProperty(KoCharacterStyle::UnderlineWeight, weight);
    d->setProperty(KoCharacterStyle::UnderlineWidth, width);
}

void KoCharacterStyle::underlineWidth(LineWeight &weight, qreal &width) const
{
    weight = (KoCharacterStyle::LineWeight) d->propertyInt(KoCharacterStyle::UnderlineWeight);
    width = d->propertyDouble(KoCharacterStyle::UnderlineWidth);
}

void KoCharacterStyle::setUnderlineMode(LineMode mode)
{
    d->setProperty(KoCharacterStyle::UnderlineMode, mode);
}

KoCharacterStyle::LineMode KoCharacterStyle::underlineMode() const
{
    return static_cast<KoCharacterStyle::LineMode>(d->propertyInt(KoCharacterStyle::UnderlineMode));
}

void KoCharacterStyle::setFontLetterSpacing(qreal spacing)
{
    d->setProperty(KoCharacterStyle::FontLetterSpacing, spacing);
}

qreal KoCharacterStyle::fontLetterSpacing() const
{
    return d->propertyDouble(KoCharacterStyle::FontLetterSpacing);
}

void KoCharacterStyle::setFontWordSpacing(qreal spacing)
{
    d->setProperty(QTextCharFormat::FontWordSpacing, spacing);
}

qreal KoCharacterStyle::fontWordSpacing() const
{
    return d->propertyDouble(QTextCharFormat::FontWordSpacing);
}


void KoCharacterStyle::setFontCapitalization(QFont::Capitalization capitalization)
{
    d->setProperty(QTextFormat::FontCapitalization, capitalization);
}

QFont::Capitalization KoCharacterStyle::fontCapitalization() const
{
    return (QFont::Capitalization) d->propertyInt(QTextFormat::FontCapitalization);
}


void KoCharacterStyle::setFontYStretch(qreal stretch)
{
    d->setProperty(KoCharacterStyle::FontYStretch, stretch);
}

qreal KoCharacterStyle::fontYStretch() const
{
    return d->propertyDouble(KoCharacterStyle::FontYStretch);
}

void KoCharacterStyle::setCountry(const QString &country)
{
    if (country.isEmpty())
        d->stylesPrivate.remove(KoCharacterStyle::Country);
    else
        d->setProperty(KoCharacterStyle::Country, country);
}

void KoCharacterStyle::setLanguage(const QString &language)
{
    if (language.isEmpty())
        d->stylesPrivate.remove(KoCharacterStyle::Language);
    else
        d->setProperty(KoCharacterStyle::Language, language);
}

QString KoCharacterStyle::country() const
{
    return value(KoCharacterStyle::Country).toString();
}

QString KoCharacterStyle::language() const
{
    return d->propertyString(KoCharacterStyle::Language);
}

bool KoCharacterStyle::blinking() const
{
    return d->propertyBoolean(Blink);
}

void KoCharacterStyle::setBlinking(bool blink)
{
    d->setProperty(KoCharacterStyle::Blink, blink);
}

bool KoCharacterStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

static QString rotationScaleToString(KoCharacterStyle::RotationScale rotationScale)
{
    QString scale = "line-height";
    if (rotationScale == KoCharacterStyle::Fixed) {
        scale = "fixed";
    }
    return scale;
}

static KoCharacterStyle::RotationScale stringToRotationScale(const QString &scale)
{
    KoCharacterStyle::RotationScale rotationScale = KoCharacterStyle::LineHeight;
    if (scale == "fixed") {
        rotationScale = KoCharacterStyle::Fixed;
    }
    return rotationScale;
}

void KoCharacterStyle::setTextRotationAngle(qreal angle)
{
    d->setProperty(TextRotationAngle, angle);
}

qreal KoCharacterStyle::textRotationAngle() const
{
    return d->propertyDouble(TextRotationAngle);
}

void KoCharacterStyle::setTextRotationScale(RotationScale scale)
{
    d->setProperty(TextRotationScale, rotationScaleToString(scale));
}

KoCharacterStyle::RotationScale KoCharacterStyle::textRotationScale() const
{
    return stringToRotationScale(d->propertyString(TextRotationScale));
}

void KoCharacterStyle::setTextScale(int scale)
{
    d->setProperty(TextScale, scale);
}

int KoCharacterStyle::textScale() const
{
    return d->propertyInt(TextScale);
}

void KoCharacterStyle::setTextShadow(const KoShadowStyle& shadow)
{
    d->setProperty(TextShadow, qVariantFromValue<KoShadowStyle>(shadow));
}

KoShadowStyle KoCharacterStyle::textShadow() const
{
    if (hasProperty(TextShadow)) {
        QVariant shadow = value(TextShadow);
        if (shadow.canConvert<KoShadowStyle>())
            return shadow.value<KoShadowStyle>();
    }
    return KoShadowStyle();
}

void KoCharacterStyle::setTextCombine(KoCharacterStyle::TextCombineType type)
{
    d->setProperty(TextCombine, type);
}

KoCharacterStyle::TextCombineType KoCharacterStyle::textCombine() const
{
    if (hasProperty(TextCombine)) {
        return (KoCharacterStyle::TextCombineType) d->propertyInt(TextCombine);
    }
    return NoTextCombine;
}

QChar KoCharacterStyle::textCombineEndChar() const
{
    if (hasProperty(TextCombineEndChar)) {
        QString val = d->propertyString(TextCombineEndChar);
        if (val.length() > 0)
            return val.at(0);
    }
    return QChar();
}

void KoCharacterStyle::setTextCombineEndChar(const QChar& character)
{
    d->setProperty(TextCombineEndChar, character);
}

QChar KoCharacterStyle::textCombineStartChar() const
{
    if (hasProperty(TextCombineStartChar)) {
        QString val = d->propertyString(TextCombineStartChar);
        if (val.length() > 0)
            return val.at(0);
    }
    return QChar();
}

void KoCharacterStyle::setTextCombineStartChar(const QChar& character)
{
    d->setProperty(TextCombineStartChar, character);
}

void KoCharacterStyle::setFontRelief(KoCharacterStyle::ReliefType relief)
{
    d->setProperty(FontRelief, relief);
}

KoCharacterStyle::ReliefType KoCharacterStyle::fontRelief() const
{
    if (hasProperty(FontRelief))
        return (KoCharacterStyle::ReliefType) d->propertyInt(FontRelief);
    return KoCharacterStyle::NoRelief;
}


KoCharacterStyle::EmphasisPosition KoCharacterStyle::textEmphasizePosition() const
{
    if (hasProperty(TextEmphasizePosition))
        return (KoCharacterStyle::EmphasisPosition) d->propertyInt(TextEmphasizePosition);
    return KoCharacterStyle::EmphasisAbove;
}

void KoCharacterStyle::setTextEmphasizePosition(KoCharacterStyle::EmphasisPosition position)
{
    d->setProperty(TextEmphasizePosition, position);
}

KoCharacterStyle::EmphasisStyle KoCharacterStyle::textEmphasizeStyle() const
{
    if (hasProperty(TextEmphasizeStyle))
        return (KoCharacterStyle::EmphasisStyle) d->propertyInt(TextEmphasizeStyle);
    return KoCharacterStyle::NoEmphasis;
}

void KoCharacterStyle::setTextEmphasizeStyle(KoCharacterStyle::EmphasisStyle emphasis)
{
    d->setProperty(TextEmphasizeStyle, emphasis);
}

void KoCharacterStyle::setPercentageFontSize(qreal percent)
{
    d->setProperty(KoCharacterStyle::PercentageFontSize, percent);
}

qreal KoCharacterStyle::percentageFontSize() const
{
    return d->propertyDouble(KoCharacterStyle::PercentageFontSize);
}

void KoCharacterStyle::setAdditionalFontSize(qreal percent)
{
    d->setProperty(KoCharacterStyle::AdditionalFontSize, percent);
}

qreal KoCharacterStyle::additionalFontSize() const
{
    return d->propertyDouble(KoCharacterStyle::AdditionalFontSize);
}

void KoCharacterStyle::loadOdf(const KoXmlElement *element, KoShapeLoadingContext &scontext,
    bool loadParents)
{
    KoOdfLoadingContext &context = scontext.odfLoadingContext();
    const QString name(element->attributeNS(KoXmlNS::style, "display-name", QString()));
    if (!name.isEmpty()) {
        d->name = name;
    }
    else {
        d->name = element->attributeNS(KoXmlNS::style, "name", QString());
    }

    QString family = element->attributeNS(KoXmlNS::style, "family", "text");

    context.styleStack().save();
    if (loadParents) {
        context.addStyles(element, family.toLocal8Bit().constData());   // Load all parent
    } else {
        context.styleStack().push(*element);
    }
    context.styleStack().setTypeProperties("text");  // load the style:text-properties
    loadOdfProperties(scontext);
    context.styleStack().restore();
}

void KoCharacterStyle::loadOdfProperties(KoShapeLoadingContext &scontext)
{
    KoStyleStack &styleStack = scontext.odfLoadingContext().styleStack();

    d->stylesPrivate = StylePrivate();

    // The fo:color attribute specifies the foreground color of text.
    const QString color(styleStack.property(KoXmlNS::fo, "color"));
    if (!color.isEmpty()) {
        QColor c(color);
        if (c.isValid()) {     // 3.10.3
            setForeground(QBrush(c));
        }
    }

    QString fontName(styleStack.property(KoXmlNS::fo, "font-family"));
    if (!fontName.isEmpty()) {
        // Specify whether a font has a fixed or variable width.
        // These attributes are ignored if there is no corresponding fo:font-family attribute attached to the same formatting properties element.
        const QString fontPitch(styleStack.property(KoXmlNS::style, "font-pitch"));
        if (!fontPitch.isEmpty()) {
            setFontFixedPitch(fontPitch == "fixed");
        }

        const QString genericFamily(styleStack.property(KoXmlNS::style, "font-family-generic"));
        if (!genericFamily.isEmpty()) {
            if (genericFamily == "roman")
                setFontStyleHint(QFont::Serif);
            else if (genericFamily == "swiss")
                setFontStyleHint(QFont::SansSerif);
            else if (genericFamily == "modern")
                setFontStyleHint(QFont::TypeWriter);
            else if (genericFamily == "decorative")
                setFontStyleHint(QFont::Decorative);
            else if (genericFamily == "system")
                setFontStyleHint(QFont::System);
            else if (genericFamily == "script") {
                ; // TODO: no hint available in Qt yet, we should at least store it as a property internally!
            }
        }

        const QString fontCharset(styleStack.property(KoXmlNS::style, "font-charset"));
        if (!fontCharset.isEmpty()) {
            // this property is not required by Qt, since Qt auto selects the right font based on the text
            // The only charset of interest to us is x-symbol - this should disable spell checking
            d->setProperty(KoCharacterStyle::FontCharset, fontCharset);
        }
    }

    const QString fontFamily(styleStack.property(KoXmlNS::style, "font-family"));
    if (!fontFamily.isEmpty())
        fontName = fontFamily;

    if (styleStack.hasProperty(KoXmlNS::style, "font-name")) {
        // This font name is a reference to a font face declaration.
        KoOdfStylesReader &stylesReader = scontext.odfLoadingContext().stylesReader();
        const KoXmlElement *fontFace = stylesReader.findStyle(styleStack.property(KoXmlNS::style, "font-name"));
        if (fontFace != 0) {
            fontName = fontFace->attributeNS(KoXmlNS::svg, "font-family", "");

            KoXmlElement fontFaceElem;
            forEachElement(fontFaceElem, (*fontFace)) {
                if (fontFaceElem.tagName() == "font-face-src") {
                    KoXmlElement fontUriElem;
                    forEachElement(fontUriElem, fontFaceElem) {
                        if (fontUriElem.tagName() == "font-face-uri") {
                            QString filename = fontUriElem.attributeNS(KoXmlNS::xlink, "href");
                            KoStore *store = scontext.odfLoadingContext().store();
                            if (store->open(filename)) {
                                KoStoreDevice device(store);
                                QByteArray data = device.readAll();
                                if (device.open(QIODevice::ReadOnly)) {
                                    QFontDatabase::addApplicationFontFromData(data);
                                }
                            }
                        }
                    }
                }
            }
        }
     }

    if (!fontName.isEmpty()) {
        // Hmm, the remove "'" could break it's in the middle of the fontname...
        fontName = fontName.remove('\'');

        // 'Thorndale' is not known outside OpenOffice so we substitute it
        // with 'Times New Roman' that looks nearly the same.
        if (fontName == "Thorndale")
            fontName = "Times New Roman";

        // 'StarSymbol' is written by OpenOffice but they actually mean
        //  'OpenSymbol'.
        if (fontName == "StarSymbol")
            fontName = "OpenSymbol";

        fontName.remove(QRegExp("\\sCE$")); // Arial CE -> Arial
        setFontFamily(fontName);
    }

    // Specify the size of a font. The value of these attribute is either an absolute length or a percentage
    if (styleStack.hasProperty(KoXmlNS::fo, "font-size")) {
        const QString fontSize(styleStack.property(KoXmlNS::fo, "font-size"));
        if (!fontSize.isEmpty()) {
            if (fontSize.endsWith('%')) {
                setPercentageFontSize(fontSize.left(fontSize.length() - 1).toDouble());
            } else {
                setFontPointSize(KoUnit::parseValue(fontSize));
            }
        }
    }
    else {
        const QString fontSizeRel(styleStack.property(KoXmlNS::style, "font-size-rel"));
        if (!fontSizeRel.isEmpty()) {
            setAdditionalFontSize(KoUnit::parseValue(fontSizeRel));
        }
    }

    // Specify the weight of a font. The permitted values are normal, bold, and numeric values 100-900, in steps of 100. Unsupported numerical values are rounded off to the next supported value.
    const QString fontWeight(styleStack.property(KoXmlNS::fo, "font-weight"));
    if (!fontWeight.isEmpty()) {     // 3.10.24
        int boldness;
        if (fontWeight == "normal")
            boldness = 50;
        else if (fontWeight == "bold")
            boldness = 75;
        else
            // XSL/CSS has 100,200,300...900. Not the same scale as Qt!
            // See http://www.w3.org/TR/2001/REC-xsl-20011015/slice7.html#font-weight
            boldness = fontWeight.toInt() / 10;
        setFontWeight(boldness);
    }

    // Specify whether to use normal or italic font face.
    const QString fontStyle(styleStack.property(KoXmlNS::fo, "font-style" ));
    if (!fontStyle.isEmpty()) {     // 3.10.19
        if (fontStyle == "italic" || fontStyle == "oblique") {    // no difference in kotext
            setFontItalic(true);
        } else {
            setFontItalic(false);
        }
    }

//TODO
#if 0
    d->m_bWordByWord = styleStack.property(KoXmlNS::style, "text-underline-mode") == "skip-white-space";
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
        // not supported by Words: "slash" and "X"
        // not supported by OO: stylelines (solid, dash, dot, dashdot, dashdotdot)
    }
    */
#endif

    // overline modes
    const QString textOverlineMode(styleStack.property( KoXmlNS::style, "text-overline-mode"));
    if (!textOverlineMode.isEmpty()) {
        if (textOverlineMode == "skip-white-space") {
            setOverlineMode(SkipWhiteSpaceLineMode);
        } else if (textOverlineMode == "continuous") {
            setOverlineMode(ContinuousLineMode);
        }
    }

    // Specifies whether text is overlined, and if so, whether a single or qreal line will be used for overlining.
    const QString textOverlineType(styleStack.property(KoXmlNS::style, "text-overline-type"));
    const QString textOverlineStyle(styleStack.property(KoXmlNS::style, "text-overline-style"));
    if (!textOverlineType.isEmpty() || !textOverlineStyle.isEmpty()) {    // OASIS 14.4.28
        LineStyle overlineStyle;
        LineType overlineType;

        importOdfLine(textOverlineType, textOverlineStyle,
                      overlineStyle, overlineType);
        setOverlineStyle(overlineStyle);
        setOverlineType(overlineType);
    }

    const QString textOverlineWidth(styleStack.property(KoXmlNS::style, "text-overline-width"));
    if (!textOverlineWidth.isEmpty()) {
        qreal overlineWidth;
        LineWeight overlineWeight;
        parseOdfLineWidth(textOverlineWidth, overlineWeight, overlineWidth);
        setOverlineWidth(overlineWeight, overlineWidth);
    }

    // Specifies the color that is used to overline text. The value of this attribute is either font-color or a color. If the value is font-color, the current text color is used for overlining.
    QString overLineColor = styleStack.property(KoXmlNS::style, "text-overline-color");   // OO 3.10.23, OASIS 14.4.31
    if (!overLineColor.isEmpty() && overLineColor != "font-color") {
        setOverlineColor(QColor(overLineColor));
    } else if (overLineColor == "font-color") {
        setOverlineColor(QColor());
    }

    // underline modes
    const QString textUndelineMode(styleStack.property( KoXmlNS::style, "text-underline-mode"));
    if (!textUndelineMode.isEmpty()) {
        if (textUndelineMode == "skip-white-space") {
            setUnderlineMode(SkipWhiteSpaceLineMode);
        } else if (textUndelineMode == "continuous") {
            setUnderlineMode(ContinuousLineMode);
        }
    }

    // Specifies whether text is underlined, and if so, whether a single or qreal line will be used for underlining.
    const QString textUnderlineType(styleStack.property(KoXmlNS::style, "text-underline-type"));
    const QString textUnderlineStyle(styleStack.property(KoXmlNS::style, "text-underline-style"));
    if (!textUnderlineType.isEmpty() || !textUnderlineStyle.isEmpty()) {    // OASIS 14.4.28
        LineStyle underlineStyle;
        LineType underlineType;

        importOdfLine(textUnderlineType, textUnderlineStyle,
                      underlineStyle, underlineType);
        setUnderlineStyle(underlineStyle);
        setUnderlineType(underlineType);
    }

    const QString textUnderlineWidth(styleStack.property(KoXmlNS::style, "text-underline-width"));
    if (!textUnderlineWidth.isEmpty()) {
        qreal underlineWidth;
        LineWeight underlineWeight;
        parseOdfLineWidth(textUnderlineWidth, underlineWeight, underlineWidth);
        setUnderlineWidth(underlineWeight, underlineWidth);
    }

    // Specifies the color that is used to underline text. The value of this attribute is either font-color or a color. If the value is font-color, the current text color is used for underlining.
    QString underLineColor = styleStack.property(KoXmlNS::style, "text-underline-color");   // OO 3.10.23, OASIS 14.4.31
    if (!underLineColor.isEmpty() && underLineColor != "font-color") {
        setUnderlineColor(QColor(underLineColor));
    } else if (underLineColor == "font-color") {
        setUnderlineColor(QColor());
    }


    const QString textLineThroughType(styleStack.property(KoXmlNS::style, "text-line-through-type"));
    const QString textLineThroughStyle(styleStack.property(KoXmlNS::style, "text-line-through-style"));
    if (!textLineThroughType.isEmpty() || !textLineThroughStyle.isEmpty()) { // OASIS 14.4.7
        KoCharacterStyle::LineStyle throughStyle;
        LineType throughType;

        importOdfLine(textLineThroughType,textLineThroughStyle,
                      throughStyle, throughType);

        setStrikeOutStyle(throughStyle);
        setStrikeOutType(throughType);
        const QString textLineThroughText(styleStack.property(KoXmlNS::style, "text-line-through-text"));
        if (!textLineThroughText.isEmpty()) {
            setStrikeOutText(textLineThroughText);
        }
    }

    const QString textLineThroughWidth(styleStack.property(KoXmlNS::style, "text-line-through-width"));
    if (!textLineThroughWidth.isEmpty()) {
        qreal throughWidth;
        LineWeight throughWeight;
        parseOdfLineWidth(textLineThroughWidth, throughWeight, throughWidth);
        setStrikeOutWidth(throughWeight, throughWidth);
    }

    const QString lineThroughColor(styleStack.property(KoXmlNS::style, "text-line-through-color"));   // OO 3.10.23, OASIS 14.4.31
    if (!lineThroughColor.isEmpty() && lineThroughColor != "font-color") {
        setStrikeOutColor(QColor(lineThroughColor));
    }

    const QString lineThroughMode(styleStack.property(KoXmlNS::style, "text-line-through-mode"));
    if (lineThroughMode == "continuous") {
        setStrikeOutMode(ContinuousLineMode);
    }
    else if (lineThroughMode == "skip-white-space") {
        setStrikeOutMode(SkipWhiteSpaceLineMode);
    }

    const QString textPosition(styleStack.property(KoXmlNS::style, "text-position"));
    if (!textPosition.isEmpty()) {  // OO 3.10.7
        if (textPosition.startsWith("super"))
            setVerticalAlignment(QTextCharFormat::AlignSuperScript);
        else if (textPosition.startsWith("sub"))
            setVerticalAlignment(QTextCharFormat::AlignSubScript);
        else {
            QRegExp re("(-?[\\d.]+)%.*");
            if (re.exactMatch(textPosition)) {
                int percent = re.capturedTexts()[1].toInt();
                if (percent > 0)
                    setVerticalAlignment(QTextCharFormat::AlignSuperScript);
                else if (percent < 0)
                    setVerticalAlignment(QTextCharFormat::AlignSubScript);
                else // set explicit to overwrite inherited text-position's
                    setVerticalAlignment(QTextCharFormat::AlignNormal);
            }
        }
    }

    // The fo:font-variant attribute provides the option to display text as small capitalized letters.
    const QString textVariant(styleStack.property(KoXmlNS::fo, "font-variant"));
    if (!textVariant.isEmpty()) {
        if (textVariant == "small-caps")
            setFontCapitalization(QFont::SmallCaps);
        else if (textVariant == "normal")
            setFontCapitalization(QFont::MixedCase);
    }
    // The fo:text-transform attribute specifies text transformations to uppercase, lowercase, and capitalization.
    else {
        const QString textTransform(styleStack.property(KoXmlNS::fo, "text-transform"));
        if (!textTransform.isEmpty()) {
            if (textTransform == "uppercase")
                setFontCapitalization(QFont::AllUppercase);
            else if (textTransform == "lowercase")
                setFontCapitalization(QFont::AllLowercase);
            else if (textTransform == "capitalize")
                setFontCapitalization(QFont::Capitalize);
            else if (textTransform == "none")
                setFontCapitalization(QFont::MixedCase);
        }
    }

    const QString foLanguage(styleStack.property(KoXmlNS::fo, "language"));
    if (!foLanguage.isEmpty()) {
        setLanguage(foLanguage);
    }

    const QString foCountry(styleStack.property(KoXmlNS::fo, "country"));
    if (!foCountry.isEmpty()) {
        setCountry(foCountry);
    }

    // The fo:background-color attribute specifies the background color of a paragraph.
    const QString bgcolor(styleStack.property(KoXmlNS::fo, "background-color"));
    if (!bgcolor.isEmpty()) {
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
    const QString useWindowFont(styleStack.property(KoXmlNS::style, "use-window-font-color"));
    if (!useWindowFont.isEmpty()) {
        setFontAutoColor(useWindowFont == "true");
    }

    const QString letterKerning(styleStack.property( KoXmlNS::style, "letter-kerning"));
    if (!letterKerning.isEmpty()) {
        setFontKerning(letterKerning == "true");
    }

    const QString letterSpacing(styleStack.property(KoXmlNS::fo, "letter-spacing"));
    if ((!letterSpacing.isEmpty()) && (letterSpacing != "normal")) {
        qreal space = KoUnit::parseValue(letterSpacing);
        setFontLetterSpacing(space);
    }

    const QString textOutline(styleStack.property(KoXmlNS::style, "text-outline"));
    if (!textOutline.isEmpty()) {
        if (textOutline == "true") {
            setTextOutline(QPen((foreground().style() != Qt::NoBrush)?foreground():QBrush(Qt::black) , 0));
            setForeground(Qt::transparent);
        } else {
            setTextOutline(QPen(Qt::NoPen));
        }
    }

    const QString textRotationAngle(styleStack.property(KoXmlNS::style, "text-rotation-angle"));
    if (!textRotationAngle.isEmpty()) {
        setTextRotationAngle(KoUnit::parseAngle(textRotationAngle));
    }

    const QString textRotationScale(styleStack.property(KoXmlNS::style, "text-rotation-scale"));
    if (!textRotationScale.isEmpty()) {
        setTextRotationScale(stringToRotationScale(textRotationScale));
    }

    const QString textScale(styleStack.property(KoXmlNS::style, "text-scale"));
    if (!textScale.isEmpty()) {
        const int scale = (textScale.endsWith('%') ? textScale.left(textScale.length()-1) : textScale).toInt();
        setTextScale(scale);
    }

    const QString textShadow(styleStack.property(KoXmlNS::fo, "text-shadow"));
    if (!textShadow.isEmpty()) {
        KoShadowStyle shadow;
        if (shadow.loadOdf(textShadow))
            setTextShadow(shadow);
    }

    const QString textCombine(styleStack.property(KoXmlNS::style, "text-combine"));
    if (!textCombine.isEmpty()) {
        if (textCombine == "letters")
            setTextCombine(TextCombineLetters);
        else if (textCombine == "lines")
            setTextCombine(TextCombineLines);
        else if (textCombine == "none")
            setTextCombine(NoTextCombine);
    }

    const QString textCombineEndChar(styleStack.property(KoXmlNS::style, "text-combine-end-char"));
    if (!textCombineEndChar.isEmpty()) {
        setTextCombineEndChar(textCombineEndChar.at(0));
    }
    const QString textCombineStartChar(styleStack.property(KoXmlNS::style, "text-combine-start-char"));
    if (!textCombineStartChar.isEmpty()) {
        setTextCombineStartChar(textCombineStartChar.at(0));
    }


    const QString fontRelief(styleStack.property(KoXmlNS::style, "font-relief"));
    if (!fontRelief.isEmpty()) {
        if (fontRelief == "none")
            setFontRelief(KoCharacterStyle::NoRelief);
        else if (fontRelief == "embossed")
            setFontRelief(KoCharacterStyle::Embossed);
        else if (fontRelief == "engraved")
            setFontRelief(KoCharacterStyle::Engraved);
    }

    const QString fontEmphasize(styleStack.property(KoXmlNS::style, "text-emphasize"));
    if (!fontEmphasize.isEmpty()) {
        QString style, position;
        QStringList parts = fontEmphasize.split(' ');
        style = parts[0];
        if (parts.length() > 1)
            position = parts[1];

        if (style == "none") {
            setTextEmphasizeStyle(NoEmphasis);
        } else if (style == "accent") {
            setTextEmphasizeStyle(AccentEmphasis);
        } else if (style == "circle") {
            setTextEmphasizeStyle(CircleEmphasis);
        } else if (style == "disc") {
            setTextEmphasizeStyle(DiscEmphasis);
        } else if (style == "dot") {
            setTextEmphasizeStyle(DotEmphasis);
        }

        if (position == "below") {
            setTextEmphasizePosition(EmphasisBelow);
        } else if (position == "above") {
            setTextEmphasizePosition(EmphasisAbove);
        }
    }

    if (styleStack.hasProperty(KoXmlNS::fo, "hyphenate"))
        setHasHyphenation(styleStack.property(KoXmlNS::fo, "hyphenate") == "true");

    if (styleStack.hasProperty(KoXmlNS::fo, "hyphenation-remain-char-count")) {
        bool ok = false;
        int count = styleStack.property(KoXmlNS::fo, "hyphenation-remain-char-count").toInt(&ok);
        if (ok)
            setHyphenationRemainCharCount(count);
    }
    if (styleStack.hasProperty(KoXmlNS::fo, "hyphenation-push-char-count")) {
        bool ok = false;
        int count = styleStack.property(KoXmlNS::fo, "hyphenation-push-char-count").toInt(&ok);
        if (ok)
            setHyphenationPushCharCount(count);
    }

    if (styleStack.hasProperty(KoXmlNS::style, "text-blinking")) {
        setBlinking(styleStack.property(KoXmlNS::style, "text-blinking") == "true");
    }


//TODO
#if 0
    /*
      Missing properties:
      style:font-style-name, 3.10.11 - can be ignored, says DV, the other ways to specify a font are more precise
      fo:letter-spacing, 3.10.16 - not implemented in kotext
      style:text-relief, 3.10.20 - not implemented in kotext
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

bool KoCharacterStyle::operator==(const KoCharacterStyle &other) const
{
     return compareCharacterProperties(other);
}

bool KoCharacterStyle::operator!=(const KoCharacterStyle &other) const
{
     return !compareCharacterProperties(other);
}

bool KoCharacterStyle::compareCharacterProperties(const KoCharacterStyle &other) const
{
    return other.d->stylesPrivate == d->stylesPrivate;
}

void KoCharacterStyle::removeDuplicates(const KoCharacterStyle &other)
{
    // In case the current style doesn't have the flag UseWindowFontColor set but the other has it set and they use the same color
    // remove duplicates will remove the color. However to make it work correctly we need to store the color with the style so it
    // will be loaded again. We don't store a use-window-font-color="false" as that is not compatible to the way OO/LO does work.
    // So save the color and restore it after the remove duplicates
    QBrush brush;
    if (other.d->propertyBoolean(KoCharacterStyle::UseWindowFontColor) && !d->propertyBoolean(KoCharacterStyle::UseWindowFontColor)) {
        brush = foreground();
    }

    // this properties should need to be kept if there is a font family defined as these are only evaluated if there is also a font family
    int keepProperties[] = { QTextFormat::FontStyleHint, QTextFormat::FontFixedPitch, KoCharacterStyle::FontCharset };

    QMap<int, QVariant> keep;
    for (unsigned int i = 0; i < sizeof(keepProperties)/sizeof(*keepProperties); ++i) {
        if (hasProperty(keepProperties[i])) {
            keep.insert(keepProperties[i], value(keepProperties[i]));
        }
    }
    this->d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
    if (brush.style() != Qt::NoBrush) {
        setForeground(brush);
    }

    // in case the char style has any of the following properties it also needs to have the fontFamily as otherwise
    // these values will be ignored when loading according to the odf spec
    if (!hasProperty(QTextFormat::FontFamily)) {
        if (hasProperty(QTextFormat::FontStyleHint) || hasProperty(QTextFormat::FontFixedPitch) || hasProperty(KoCharacterStyle::FontCharset)) {
            QString fontFamily = other.fontFamily();
            if (!fontFamily.isEmpty()) {
                setFontFamily(fontFamily);
            }
        }
    }
    else {
        for (QMap<int, QVariant>::const_iterator it(keep.constBegin()); it != keep.constEnd(); ++it) {
            this->d->stylesPrivate.add(it.key(), it.value());
        }
    }
}

void KoCharacterStyle::removeDuplicates(const QTextCharFormat &otherFormat)
{
    KoCharacterStyle other(otherFormat);
    removeDuplicates(other);
}

void KoCharacterStyle::remove(int key)
{
    d->stylesPrivate.remove(key);
}

bool KoCharacterStyle::isEmpty() const
{
    return d->stylesPrivate.isEmpty();
}

void KoCharacterStyle::saveOdf(KoGenStyle &style) const
{
    if (!d->name.isEmpty() && !style.isDefaultStyle()) {
        style.addAttribute("style:display-name", d->name);
    }
    QList<int> keys = d->stylesPrivate.keys();
    Q_FOREACH (int key, keys) {
        if (key == QTextFormat::FontWeight) {
            bool ok = false;
            int boldness = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                if (boldness == QFont::Normal) {
                    style.addProperty("fo:font-weight", "normal", KoGenStyle::TextType);
                } else if (boldness == QFont::Bold) {
                    style.addProperty("fo:font-weight", "bold", KoGenStyle::TextType);
                } else {
                    // Remember : Qt and CSS/XSL doesn't have the same scale. Its 100-900 instead of Qts 0-100
                    style.addProperty("fo:font-weight", qBound(10, boldness, 90) * 10, KoGenStyle::TextType);
                }
            }
        } else if (key == QTextFormat::FontItalic) {
            if (d->stylesPrivate.value(key).toBool()) {
                style.addProperty("fo:font-style", "italic", KoGenStyle::TextType);
            } else {
                style.addProperty("fo:font-style", "normal", KoGenStyle::TextType);
            }
        } else if (key == QTextFormat::FontFamily) {
            QString fontFamily = d->stylesPrivate.value(key).toString();
            style.addProperty("fo:font-family", fontFamily, KoGenStyle::TextType);
        } else if (key == QTextFormat::FontFixedPitch) {
            bool fixedPitch = d->stylesPrivate.value(key).toBool();
            style.addProperty("style:font-pitch", fixedPitch ? "fixed" : "variable", KoGenStyle::TextType);
            // if this property is saved we also need to save the fo:font-family attribute as otherwise it will be ignored on loading as defined in the spec
            style.addProperty("fo:font-family", fontFamily(), KoGenStyle::TextType);
        } else if (key == QTextFormat::FontStyleHint) {
            bool ok = false;
            int styleHint = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                QString generic = exportOdfFontStyleHint((QFont::StyleHint) styleHint);
                if (!generic.isEmpty()) {
                    style.addProperty("style:font-family-generic", generic, KoGenStyle::TextType);
                }
                // if this property is saved we also need to save the fo:font-family attribute as otherwise it will be ignored on loading as defined in the spec
                style.addProperty("fo:font-family", fontFamily(), KoGenStyle::TextType);
            }
        } else if (key == QTextFormat::FontKerning) {
            style.addProperty("style:letter-kerning", fontKerning() ? "true" : "false", KoGenStyle::TextType);
        } else if (key == QTextFormat::FontCapitalization) {
            switch (fontCapitalization()) {
            case QFont::SmallCaps:
                style.addProperty("fo:font-variant", "small-caps", KoGenStyle::TextType);
                break;
            case QFont::MixedCase:
                style.addProperty("fo:font-variant", "normal", KoGenStyle::TextType);
                style.addProperty("fo:text-transform", "none", KoGenStyle::TextType);
                break;
            case QFont::AllUppercase:
                style.addProperty("fo:text-transform", "uppercase", KoGenStyle::TextType);
                break;
            case QFont::AllLowercase:
                style.addProperty("fo:text-transform", "lowercase", KoGenStyle::TextType);
                break;
            case QFont::Capitalize:
                style.addProperty("fo:text-transform", "capitalize", KoGenStyle::TextType);
                break;
            }
        } else if (key == OverlineStyle) {
            bool ok = false;
            int styleId = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                style.addProperty("style:text-overline-style", exportOdfLineStyle((KoCharacterStyle::LineStyle) styleId), KoGenStyle::TextType);
        }
        } else if (key == OverlineType) {
            bool ok = false;
            int type = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                style.addProperty("style:text-overline-type", exportOdfLineType((KoCharacterStyle::LineType) type), KoGenStyle::TextType);
        }
        } else if (key == OverlineColor) {
            QColor color = d->stylesPrivate.value(key).value<QColor>();
            if (color.isValid())
                style.addProperty("style:text-overline-color", color.name(), KoGenStyle::TextType);
            else
                style.addProperty("style:text-overline-color", "font-color", KoGenStyle::TextType);
        } else if (key == OverlineMode) {
            bool ok = false;
            int mode = d->stylesPrivate.value(key).toInt(&ok);
            if (ok) {
                style.addProperty("style:text-overline-mode", exportOdfLineMode((KoCharacterStyle::LineMode) mode), KoGenStyle::TextType);
        }
        } else if (key == OverlineWidth) {
            KoCharacterStyle::LineWeight weight;
            qreal width;
            overlineWidth(weight, width);
            style.addProperty("style:text-overline-width", exportOdfLineWidth(weight, width), KoGenStyle::TextType);
        } else if (key == UnderlineStyle) {
            bool ok = false;
            int styleId = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-underline-style", exportOdfLineStyle((KoCharacterStyle::LineStyle) styleId), KoGenStyle::TextType);
        } else if (key == UnderlineType) {
            bool ok = false;
            int type = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-underline-type", exportOdfLineType((KoCharacterStyle::LineType) type), KoGenStyle::TextType);
        } else if (key == QTextFormat::TextUnderlineColor) {
            QColor color = d->stylesPrivate.value(key).value<QColor>();
            if (color.isValid())
                style.addProperty("style:text-underline-color", color.name(), KoGenStyle::TextType);
            else
                style.addProperty("style:text-underline-color", "font-color", KoGenStyle::TextType);
        } else if (key == UnderlineMode) {
            bool ok = false;
            int mode = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-underline-mode", exportOdfLineMode((KoCharacterStyle::LineMode) mode), KoGenStyle::TextType);
        } else if (key == UnderlineWidth) {
            KoCharacterStyle::LineWeight weight;
            qreal width;
            underlineWidth(weight, width);
            style.addProperty("style:text-underline-width", exportOdfLineWidth(weight, width), KoGenStyle::TextType);
        } else if (key == StrikeOutStyle) {
            bool ok = false;
            int styleId = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-line-through-style", exportOdfLineStyle((KoCharacterStyle::LineStyle) styleId), KoGenStyle::TextType);
        } else if (key == StrikeOutType) {
            bool ok = false;
            int type = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-line-through-type", exportOdfLineType((KoCharacterStyle::LineType) type), KoGenStyle::TextType);
        } else if (key == StrikeOutText) {
            style.addProperty("style:text-line-through-text", d->stylesPrivate.value(key).toString(), KoGenStyle::TextType);
        } else if (key == StrikeOutColor) {
            QColor color = d->stylesPrivate.value(key).value<QColor>();
            if (color.isValid())
                style.addProperty("style:text-line-through-color", color.name(), KoGenStyle::TextType);
        } else if (key == StrikeOutMode) {
            bool ok = false;
            int mode = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:text-line-through-mode", exportOdfLineMode((KoCharacterStyle::LineMode) mode), KoGenStyle::TextType);
        } else if (key == StrikeOutWidth) {
            KoCharacterStyle::LineWeight weight;
            qreal width;
            strikeOutWidth(weight, width);
            style.addProperty("style:text-line-through-width", exportOdfLineWidth(weight, width), KoGenStyle::TextType);
        } else if (key == QTextFormat::BackgroundBrush) {
            QBrush brush = d->stylesPrivate.value(key).value<QBrush>();
            if (brush.style() == Qt::NoBrush)
                style.addProperty("fo:background-color", "transparent", KoGenStyle::TextType);
            else
                style.addProperty("fo:background-color", brush.color().name(), KoGenStyle::TextType);
        } else if (key == QTextFormat::ForegroundBrush) {
            QBrush brush = d->stylesPrivate.value(key).value<QBrush>();
            if (brush.style() != Qt::NoBrush) {
                style.addProperty("fo:color", brush.color().name(), KoGenStyle::TextType);
            }
        } else if (key == KoCharacterStyle::UseWindowFontColor) {
            bool use = d->stylesPrivate.value(key).toBool();
            style.addProperty("style:use-window-font-color", use ? "true" : "false", KoGenStyle::TextType);
        } else if (key == QTextFormat::TextVerticalAlignment) {
            if (verticalAlignment() == QTextCharFormat::AlignSuperScript)
                style.addProperty("style:text-position", "super", KoGenStyle::TextType);
            else if (verticalAlignment() == QTextCharFormat::AlignSubScript)
                style.addProperty("style:text-position", "sub", KoGenStyle::TextType);
            else if (d->stylesPrivate.contains(QTextFormat::TextVerticalAlignment)) // no superscript or subscript
                style.addProperty("style:text-position", "0% 100%", KoGenStyle::TextType);
        } else if (key == QTextFormat::FontPointSize) {
            // when there is percentageFontSize!=100% property ignore the fontSize property and store the percentage property
            if ( (!hasProperty(KoCharacterStyle::PercentageFontSize)) || (percentageFontSize()==100))
                style.addPropertyPt("fo:font-size", fontPointSize(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::PercentageFontSize) {
            if(percentageFontSize()!=100) {
                style.addProperty("fo:font-size", QString::number(percentageFontSize()) + '%', KoGenStyle::TextType);
            }
        } else if (key == KoCharacterStyle::Country) {
            style.addProperty("fo:country", d->stylesPrivate.value(KoCharacterStyle::Country).toString(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::Language) {
            style.addProperty("fo:language", d->stylesPrivate.value(KoCharacterStyle::Language).toString(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::FontLetterSpacing) {
            qreal space = fontLetterSpacing();
            style.addPropertyPt("fo:letter-spacing", space, KoGenStyle::TextType);
        } else if (key == QTextFormat::TextOutline) {
            QPen outline = textOutline();
            style.addProperty("style:text-outline", outline.style() == Qt::NoPen ? "false" : "true", KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::FontCharset) {
            style.addProperty("style:font-charset", d->stylesPrivate.value(KoCharacterStyle::FontCharset).toString(), KoGenStyle::TextType);
            // if this property is saved we also need to save the fo:font-family attribute as otherwise it will be ignored on loading as defined in the spec
            style.addProperty("fo:font-family", fontFamily(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::TextRotationAngle) {
            style.addProperty("style:text-rotation-angle", QString::number(textRotationAngle()), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::TextRotationScale) {
            RotationScale scale = textRotationScale();
            style.addProperty("style:text-rotation-scale", rotationScaleToString(scale), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::TextScale) {
            int scale = textScale();
            style.addProperty("style:text-scale", QString::number(scale) + '%', KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::TextShadow) {
            KoShadowStyle shadow = textShadow();
            style.addProperty("fo:text-shadow", shadow.saveOdf(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::TextCombine) {
            KoCharacterStyle::TextCombineType textCombineType = textCombine();
            switch (textCombineType)
            {
                case KoCharacterStyle::NoTextCombine:
                    style.addProperty("style:text-combine", "none", KoGenStyle::TextType);
                    break;
                case KoCharacterStyle::TextCombineLetters:
                    style.addProperty("style:text-combine", "letters", KoGenStyle::TextType);
                    break;
                case KoCharacterStyle::TextCombineLines:
                    style.addProperty("style:text-combine", "lines", KoGenStyle::TextType);
                    break;
            }
        } else if (key == KoCharacterStyle::TextCombineEndChar) {
            style.addProperty("style:text-combine-end-char", textCombineEndChar(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::TextCombineStartChar) {
            style.addProperty("style:text-combine-start-char", textCombineStartChar(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::FontRelief) {
            KoCharacterStyle::ReliefType relief = fontRelief();
            switch (relief)
            {
                case KoCharacterStyle::NoRelief:
                    style.addProperty("style:font-relief", "none", KoGenStyle::TextType);
                    break;
                case KoCharacterStyle::Embossed:
                    style.addProperty("style:font-relief", "embossed", KoGenStyle::TextType);
                    break;
                case KoCharacterStyle::Engraved:
                    style.addProperty("style:font-relief", "engraved", KoGenStyle::TextType);
                    break;
            }
        } else if (key == KoCharacterStyle::TextEmphasizeStyle) {
            KoCharacterStyle::EmphasisStyle emphasisStyle = textEmphasizeStyle();
            KoCharacterStyle::EmphasisPosition position = textEmphasizePosition();
            QString odfEmphasis;
            switch (emphasisStyle)
            {
                case KoCharacterStyle::NoEmphasis:
                    odfEmphasis = "none";
                    break;
                case KoCharacterStyle::AccentEmphasis:
                    odfEmphasis = "accent";
                    break;
                case KoCharacterStyle::CircleEmphasis:
                    odfEmphasis = "circle";
                    break;
                case KoCharacterStyle::DiscEmphasis:
                    odfEmphasis = "disc";
                    break;
                case KoCharacterStyle::DotEmphasis:
                    odfEmphasis = "dot";
                    break;
            }
            if (hasProperty(KoCharacterStyle::TextEmphasizePosition)) {
                if (position == KoCharacterStyle::EmphasisAbove)
                    odfEmphasis += " above";
                else
                    odfEmphasis += " below";
            }
            style.addProperty("style:text-emphasize", odfEmphasis, KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::HasHyphenation) {
            if (hasHyphenation())
                style.addProperty("fo:hyphenate", "true", KoGenStyle::TextType);
            else
                style.addProperty("fo:hyphenate", "false", KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::HyphenationPushCharCount) {
            style.addProperty("fo:hyphenation-push-char-count", hyphenationPushCharCount(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::HyphenationRemainCharCount) {
            style.addProperty("fo:hyphenation-remain-char-count", hyphenationRemainCharCount(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::Blink) {
            style.addProperty("style:text-blinking", blinking(), KoGenStyle::TextType);
        }
    }
    //TODO: font name and family
}

QVariant KoCharacterStyle::value(int key) const
{
    QVariant variant = d->stylesPrivate.value(key);
    if (variant.isNull()) {
        if (d->parentStyle)
            variant = d->parentStyle->value(key);
        else if (d->defaultStyle)
            variant = d->defaultStyle->value(key);
    }
    return variant;
}

void KoCharacterStyle::removeHardCodedDefaults()
{
    d->hardCodedDefaultStyle.clearAll();
}
