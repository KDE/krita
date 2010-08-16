/* This file is part of the KDE project
 * Copyright (C) 2006-2009 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Sebastian Sauer <mail@dipe.org>
*  Copyright (C) 2008 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
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
#include <QFontMetrics>

#include <KoOdfLoadingContext.h>
#include <KoOdfStylesReader.h>
#include <KoXmlNS.h>
#include <KoXmlReader.h>
#include <KoUnit.h>
#include <KoGenStyle.h>

#include <KDebug>

class KoCharacterStyle::Private
{
public:
    Private();
    ~Private() { }

    void setProperty(int key, const QVariant &value) {
        stylesPrivate.add(key, value);
    }
    qreal propertyDouble(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return 0.0;
        return variant.toDouble();
    }
    int propertyInt(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return 0;
        return variant.toInt();
    }
    QString propertyString(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return QString();
        return qvariant_cast<QString>(variant);
    }
    bool propertyBoolean(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return false;
        return variant.toBool();
    }
    QColor propertyColor(int key) const {
        QVariant variant = stylesPrivate.value(key);
        if (variant.isNull())
            return QColor();
        return variant.value<QColor>();
    }

    //Overload the hard-coded default with defaultstyles.xml properties if defined
    void setApplicationDefaults(KoOdfLoadingContext &context);

    //This should be called after all charFormat properties are merged to the cursor.
    void ensureMinimalProperties(QTextCursor &cursor, bool blockCharFormatAlso);

    StylePrivate hardCodedDefaultStyle;

    QString name;
    StylePrivate stylesPrivate;
};

KoCharacterStyle::Private::Private()
{
    //set the minimal default properties
    hardCodedDefaultStyle.add(QTextFormat::FontFamily, QString("Sans Serif"));
    hardCodedDefaultStyle.add(QTextFormat::FontPointSize, 12.0);
    hardCodedDefaultStyle.add(QTextFormat::ForegroundBrush, QBrush(Qt::black));
}

void KoCharacterStyle::Private::setApplicationDefaults(KoOdfLoadingContext &context)
{
    KoStyleStack defaultStyleStack;
    const KoXmlElement *appDef = context.defaultStylesReader().defaultStyle("paragraph");
    if (appDef) {
        defaultStyleStack.push(*appDef);
        defaultStyleStack.setTypeProperties("text");
        KoCharacterStyle defStyle;
        defStyle.loadOdfProperties(defaultStyleStack);

        QList<int> keys = defStyle.d->stylesPrivate.keys();
        foreach(int key, keys) {
            hardCodedDefaultStyle.add(key, defStyle.value(key));
        }
    }
}

void KoCharacterStyle::Private::ensureMinimalProperties(QTextCursor &cursor, bool blockCharFormatAlso)
{
    QTextCharFormat format = cursor.charFormat();
    QMap<int, QVariant> props = hardCodedDefaultStyle.properties();
    QMap<int, QVariant>::const_iterator it = props.constBegin();
    while (it != props.constEnd()) {
        if (!it.value().isNull() && !!format.hasProperty(it.key())) {
            format.setProperty(it.key(), it.value());
        }
        ++it;
    }
    cursor.mergeCharFormat(format);
    if (blockCharFormatAlso)
        cursor.mergeBlockCharFormat(format);
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

void KoCharacterStyle::copyProperties(const KoCharacterStyle *style)
{
    d->stylesPrivate = style->d->stylesPrivate;
    setName(style->name()); // make sure we emit property change
}

void KoCharacterStyle::copyProperties(const QTextCharFormat &format)
{
    d->stylesPrivate = format.properties();
}

KoCharacterStyle *KoCharacterStyle::clone(QObject *parent)
{
    KoCharacterStyle *newStyle = new KoCharacterStyle(parent);
    newStyle->copyProperties(this);
    return newStyle;
}

KoCharacterStyle::~KoCharacterStyle()
{
    delete d;
}

QPen KoCharacterStyle::textOutline() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::TextOutline);
    if (variant.isNull()) {
        return QPen(Qt::NoPen);
    }
    return qvariant_cast<QPen>(variant);
}

QBrush KoCharacterStyle::background() const
{
    QVariant variant = d->stylesPrivate.value(QTextFormat::BackgroundBrush);

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
    QVariant variant = d->stylesPrivate.value(QTextFormat::ForegroundBrush);
    if (variant.isNull()) {
        return QBrush();
    }
    return qvariant_cast<QBrush>(variant);
}

void KoCharacterStyle::clearForeground()
{
    d->stylesPrivate.remove(QTextCharFormat::ForegroundBrush);
}

void KoCharacterStyle::applyStyle(QTextCharFormat &format) const
{
    const QMap<int, QVariant> props = d->stylesPrivate.properties();
    QMap<int, QVariant>::const_iterator it = props.begin();
    while (it != props.end()) {
        if (!it.value().isNull()) {
            format.setProperty(it.key(), it.value());
        }
        ++it;
    }
}

void KoCharacterStyle::applyStyle(QTextBlock &block) const
{
    QTextCursor cursor(block);
    QTextCharFormat cf;
    cursor.setPosition(block.position() + block.length() - 1, QTextCursor::KeepAnchor);
    applyStyle(cf);
    cursor.mergeCharFormat(cf);
    cursor.mergeBlockCharFormat(cf);
    d->ensureMinimalProperties(cursor, true);
}

void KoCharacterStyle::applyStyle(QTextCursor *selection) const
{
    QTextCharFormat cf;
    applyStyle(cf);
    selection->mergeCharFormat(cf);
    d->ensureMinimalProperties(*selection, false);
}

void KoCharacterStyle::unapplyStyle(QTextCharFormat &format) const
{
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
        iter--;
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
static void importOdfLine(const QString &type, const QString &style, const QString &width,
                          KoCharacterStyle::LineStyle &lineStyle, KoCharacterStyle::LineType &lineType,
                          KoCharacterStyle::LineWeight &lineWeight, qreal &lineWidth)
{
    lineStyle = KoCharacterStyle::NoLineStyle;
    lineType = KoCharacterStyle::NoLineType;
    lineWidth = 0;
    lineWeight = KoCharacterStyle::AutoLineWeight;

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
        lineWeight = KoCharacterStyle::PercentLineWeight;
        lineWidth = 100 * width.toDouble();
    } else {
        lineWeight = KoCharacterStyle::LengthLineWeight;
        lineWidth = KoUnit::parseValue(width);
    }
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
}
QString KoCharacterStyle::fontFamily() const
{
    return d->propertyString(QTextFormat::FontFamily);
}
void KoCharacterStyle::setFontPointSize(qreal size)
{
    d->setProperty(QTextFormat::FontPointSize, size);
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
void KoCharacterStyle::setFontOverline(bool overline)
{
    d->setProperty(QTextFormat::FontOverline, overline);
}
bool KoCharacterStyle::fontOverline() const
{
    return d->propertyBoolean(QTextFormat::FontOverline);
}
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
    d->setProperty(QTextCharFormat::FontLetterSpacing, spacing);
}

qreal KoCharacterStyle::fontLetterSpacing() const
{
    return d->propertyDouble(QTextCharFormat::FontLetterSpacing);
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
    return d->stylesPrivate.value(KoCharacterStyle::Country).toString();
}

QString KoCharacterStyle::language() const
{
    return d->propertyString(KoCharacterStyle::Language);
}

bool KoCharacterStyle::hasProperty(int key) const
{
    return d->stylesPrivate.contains(key);
}

static KoCharacterStyle::RotationAngle intToRotationAngle(int angle)
{
    KoCharacterStyle::RotationAngle rotationAngle = KoCharacterStyle::Zero;
    if (angle == 90) {
        rotationAngle = KoCharacterStyle::Ninety;
    } else if (angle == 270) {
        rotationAngle = KoCharacterStyle::TwoHundredSeventy;
    }
    return rotationAngle;
}

static int rotationAngleToInt(KoCharacterStyle::RotationAngle rotationAngle)
{
    int angle = 0;
    if (rotationAngle == KoCharacterStyle::Ninety) {
        angle = 90;
    } else if (rotationAngle == KoCharacterStyle::TwoHundredSeventy) {
        angle = 270;
    }
    return angle;
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

void KoCharacterStyle::setTextRotationAngle(RotationAngle angle)
{
    d->setProperty(TextRotationAngle, rotationAngleToInt(angle));
}

KoCharacterStyle::RotationAngle KoCharacterStyle::textRotationAngle() const
{
    return intToRotationAngle(d->propertyInt(TextRotationAngle));
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

//in 1.6 this was defined in KoTextFormat::load(KoOasisContext &context)
void KoCharacterStyle::loadOdf(KoOdfLoadingContext &context)
{
    d->setApplicationDefaults(context);
    KoStyleStack &styleStack = context.styleStack();
    loadOdfProperties(styleStack);

    QString fontName;
    if (styleStack.hasProperty(KoXmlNS::style, "font-name")) {
        // This font name is a reference to a font face declaration.
        KoOdfStylesReader &stylesReader = context.stylesReader();
        const KoXmlElement *fontFace = stylesReader.findStyle(styleStack.property(KoXmlNS::style, "font-name"));
        if (fontFace != 0)
            fontName = fontFace->attributeNS(KoXmlNS::svg, "font-family", "");
    }
    if (! fontName.isEmpty()) {
    // Hmm, the remove "'" could break it's in the middle of the fontname...
        fontName = fontName.remove('\'');

    // 'Thorndale' is not known outside OpenOffice so we substitute it
    // with 'Times New Roman' that looks nearly the same.
        if (fontName == "Thorndale")
            fontName = "Times New Roman";

        fontName.remove(QRegExp("\\sCE$")); // Arial CE -> Arial
        setFontFamily(fontName);
    }
}

void KoCharacterStyle::loadOdfProperties(KoStyleStack &styleStack)
{
    // The fo:color attribute specifies the foreground color of text.
    const QString color(styleStack.property(KoXmlNS::fo, "color"));
    if (!color.isEmpty()) {
        QColor c(color);
        if (c.isValid()) {     // 3.10.3
            setForeground(QBrush(c));
        }
        // if (styleStack.property(KoXmlNS::style, "use-window-font-color") == "true")
            // we should store this property to allow the layout to ignore the above set color in some situations.
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

    if (!fontName.isEmpty()) {
        // Hmm, the remove "'" could break it's in the middle of the fontname...
        fontName = fontName.remove('\'');

        // 'Thorndale' is not known outside OpenOffice so we substitute it
        // with 'Times New Roman' that looks nearly the same.
        if (fontName == "Thorndale")
            fontName = "Times New Roman";

        fontName.remove(QRegExp("\\sCE$")); // Arial CE -> Arial
        setFontFamily(fontName);
    }

    // Specify the size of a font. The value of these attribute is either an absolute length or a percentage
    if (styleStack.hasProperty(KoXmlNS::fo, "font-size")) {
        qreal pointSize = styleStack.fontSize();
        if (pointSize > 0) {
            setFontPointSize(pointSize);
        }
    }
    else {
        const QString fontSizeRel(styleStack.property(KoXmlNS::style, "font-size-rel"));
        if (!fontSizeRel.isEmpty()) {
        // These attributes specify a relative font size change as a length such as +1pt, -3pt. It changes the font size based on the font size of the parent style.
            qreal pointSize = styleStack.fontSize() + KoUnit::parseValue(fontSizeRel);
            if (pointSize > 0) {
                setFontPointSize(pointSize);
            }
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
        // not supported by KWord: "slash" and "X"
        // not supported by OO: stylelines (solid, dash, dot, dashdot, dashdotdot)
    }
    */
#endif

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
        qreal underlineWidth;
        LineWeight underlineWeight;

        importOdfLine(textUnderlineType, textUnderlineStyle,
                      styleStack.property(KoXmlNS::style, "text-underline-width"),
                      underlineStyle, underlineType, underlineWeight, underlineWidth);
        setUnderlineStyle(underlineStyle);
        setUnderlineType(underlineType);
        setUnderlineWidth(underlineWeight, underlineWidth);
    }

    // Specifies the color that is used to underline text. The value of this attribute is either font-color or a color. If the value is font-color, the current text color is used for underlining.
    QString underLineColor = styleStack.property(KoXmlNS::style, "text-underline-color");   // OO 3.10.23, OASIS 14.4.31
    if (!underLineColor.isEmpty() && underLineColor != "font-color")
        setUnderlineColor(QColor(underLineColor));


    const QString textLineThroughType(styleStack.property(KoXmlNS::style, "text-line-through-type"));
    const QString textLineThroughStyle(styleStack.property(KoXmlNS::style, "text-line-through-style"));
    if (!textLineThroughType.isEmpty() || !textLineThroughStyle.isEmpty()) { // OASIS 14.4.7
        KoCharacterStyle::LineStyle throughStyle;
        LineType throughType;
        qreal throughWidth;
        LineWeight throughWeight;

        importOdfLine(textLineThroughType,textLineThroughStyle,
                      styleStack.property(KoXmlNS::style, "text-line-through-width"),
                      throughStyle, throughType, throughWeight, throughWidth);

        setStrikeOutStyle(throughStyle);
        setStrikeOutType(throughType);
        setStrikeOutWidth(throughWeight, throughWidth);
        const QString textLineThroughText(styleStack.property(KoXmlNS::style, "text-line-through-text"));
        if (!textLineThroughText.isEmpty()) {
            setStrikeOutText(textLineThroughText);
        }
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
                float value = re.capturedTexts()[1].toFloat();
                if (value > 0)
                    setVerticalAlignment(QTextCharFormat::AlignSuperScript);
                else if (value < 0)
                    setVerticalAlignment(QTextCharFormat::AlignSubScript);
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
    if (styleStack.property(KoXmlNS::style, "use-window-font-color") == "true") {
        // Do like OpenOffice.org : change the foreground font if its color is too close to the background color...

        QColor back = background().color();
        QColor front = foreground().color();
        if ((abs(qGray(back.rgb()) - qGray(front.rgb())) < 10) && (background().style() != Qt::NoBrush)) {
            front.setRed(255 - front.red());
            front.setGreen(255 - front.green());
            front.setBlue(255 - front.blue());
            QBrush frontBrush = foreground();
            frontBrush.setColor(front);
            if (frontBrush.style() == Qt::NoBrush) {
                frontBrush.setStyle(Qt::SolidPattern);
            }
            setForeground(frontBrush);
        }
    }

    const QString letterKerning(styleStack.property( KoXmlNS::style, "letter-kerning"));
    if (!letterKerning.isEmpty()) {
        setFontKerning(letterKerning == "true");
    }

    const QString letterSpacing(styleStack.property(KoXmlNS::fo, "letter-spacing"));
    if (!letterSpacing.isEmpty()) {
        qreal space = KoUnit::parseValue(letterSpacing);
        QFontMetrics fm(font());
        setFontLetterSpacing(100+100*space/fm.averageCharWidth());
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
        int angle = textRotationAngle.toInt();
        setTextRotationAngle(intToRotationAngle(angle));
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

//TODO
#if 0
    if (styleStack.hasProperty(KoXmlNS::fo, "text-shadow")) {    // 3.10.21
        parseShadowFromCss(styleStack.property(KoXmlNS::fo, "text-shadow"));
    }

    d->m_bHyphenation = true;
    if (styleStack.hasProperty(KoXmlNS::fo, "hyphenate"))     // it's a character property in OASIS (but not in OO-1.1)
        d->m_bHyphenation = styleStack.property(KoXmlNS::fo, "hyphenate") == "true";

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
    return other.d->stylesPrivate == d->stylesPrivate;
}

void KoCharacterStyle::removeDuplicates(const KoCharacterStyle &other)
{
    this->d->stylesPrivate.removeDuplicates(other.d->stylesPrivate);
}

void KoCharacterStyle::removeDuplicates(const QTextCharFormat &otherFormat)
{
    KoCharacterStyle other(otherFormat);
    removeDuplicates(other);
}

bool KoCharacterStyle::isEmpty() const
{
    return d->stylesPrivate.isEmpty();
}

void KoCharacterStyle::saveOdf(KoGenStyle &style)
{
    if (!d->name.isEmpty() && !style.isDefaultStyle()) {
        style.addAttribute("style:display-name", d->name);
    }
    QList<int> keys = d->stylesPrivate.keys();
    foreach(int key, keys) {
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
                style.addProperty("fo:font-style", "", KoGenStyle::TextType);
            }
        } else if (key == QTextFormat::FontFamily) {
            QString fontFamily = d->stylesPrivate.value(key).toString();
            style.addProperty("fo:font-family", fontFamily, KoGenStyle::TextType);
        } else if (key == QTextFormat::FontFixedPitch) {
            bool fixedPitch = d->stylesPrivate.value(key).toBool();
            style.addProperty("style:font-pitch", fixedPitch ? "fixed" : "variable", KoGenStyle::TextType);
        } else if (key == QTextFormat::FontStyleHint) {
            bool ok = false;
            int styleHint = d->stylesPrivate.value(key).toInt(&ok);
            if (ok)
                style.addProperty("style:font-family-generic", exportOdfFontStyleHint((QFont::StyleHint) styleHint), KoGenStyle::TextType);
        } else if (key == QTextFormat::FontKerning) {
            style.addProperty("style:letter-kerning", fontKerning() ? "true" : "false", KoGenStyle::TextType);
        } else if (key == QTextFormat::FontCapitalization) {
            switch (fontCapitalization()) {
            case QFont::SmallCaps:
                style.addProperty("fo:font-variant", "small-caps", KoGenStyle::TextType);
                break;
            case QFont::MixedCase:
                style.addProperty("fo:font-variant", "normal", KoGenStyle::TextType);
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
            if (brush.style() == Qt::NoBrush)
                style.addProperty("fo:color", "transparent", KoGenStyle::TextType);
            else
                style.addProperty("fo:color", brush.color().name(), KoGenStyle::TextType);
        } else if (key == QTextFormat::TextVerticalAlignment) {
            if (verticalAlignment() == QTextCharFormat::AlignSuperScript)
                style.addProperty("style:text-position", "super", KoGenStyle::TextType);
            else if (verticalAlignment() == QTextCharFormat::AlignSubScript)
                style.addProperty("style:text-position", "sub", KoGenStyle::TextType);
        } else if (key == QTextFormat::FontPointSize) {
            style.addPropertyPt("fo:font-size", fontPointSize(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::Country) {
            style.addProperty("fo:country", d->stylesPrivate.value(KoCharacterStyle::Country).toString(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::Language) {
            style.addProperty("fo:language", d->stylesPrivate.value(KoCharacterStyle::Language).toString(), KoGenStyle::TextType);
        } else if (key == QTextCharFormat::FontLetterSpacing) {
            style.addProperty("fo:letter-spacing", (int) fontLetterSpacing(), KoGenStyle::TextType);
        } else if (key == QTextFormat::TextOutline) {
            QPen outline = textOutline();
            style.addProperty("style:text-outline", outline.style() == Qt::NoPen ? "false" : "true", KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::FontCharset) {
            style.addProperty("style:font-charset", d->stylesPrivate.value(KoCharacterStyle::FontCharset).toString(), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::TextRotationAngle) {
            RotationAngle angle = textRotationAngle();
            style.addProperty("style:text-rotation-angle", rotationAngleToInt(angle), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::TextRotationScale) {
            RotationScale scale = textRotationScale();
            style.addProperty("style:text-rotation-scale", rotationScaleToString(scale), KoGenStyle::TextType);
        } else if (key == KoCharacterStyle::TextScale) {
            int scale = textScale();
            style.addProperty("style:text-scale", QString::number(scale) + '%', KoGenStyle::TextType);
        }
    }
    //TODO: font name and family
}

QVariant KoCharacterStyle::value(int key) const
{
    return d->stylesPrivate.value(key);
}

void KoCharacterStyle::removeHardCodedDefaults()
{
    d->hardCodedDefaultStyle.clearAll();
}

#include <KoCharacterStyle.moc>
