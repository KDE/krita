/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextProperties.h"

#include <QMap>
#include "KoSvgText.h"
#include <SvgUtil.h>
#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <QFontMetrics>
#include <QGlobalStatic>
#include "kis_global.h"
#include "kis_dom_utils.h"

struct KoSvgTextProperties::Private
{
    QMap<PropertyId, QVariant> properties;

    static bool isInheritable(PropertyId id);
};

KoSvgTextProperties::KoSvgTextProperties()
    : m_d(new Private)
{
}

KoSvgTextProperties::~KoSvgTextProperties()
{
}

KoSvgTextProperties::KoSvgTextProperties(const KoSvgTextProperties &rhs)
    : m_d(new Private)
{
    m_d->properties = rhs.m_d->properties;
}

KoSvgTextProperties &KoSvgTextProperties::operator=(const KoSvgTextProperties &rhs)
{
    if (&rhs != this) {
        m_d->properties = rhs.m_d->properties;
    }
    return *this;
}

void KoSvgTextProperties::setProperty(KoSvgTextProperties::PropertyId id, const QVariant &value)
{
    m_d->properties.insert(id, value);
}

bool KoSvgTextProperties::hasProperty(KoSvgTextProperties::PropertyId id) const
{
    return m_d->properties.contains(id);
}

QVariant KoSvgTextProperties::property(KoSvgTextProperties::PropertyId id, const QVariant &defaultValue) const
{
    return m_d->properties.value(id, defaultValue);
}

void KoSvgTextProperties::removeProperty(KoSvgTextProperties::PropertyId id)
{
    m_d->properties.remove(id);
}

QVariant KoSvgTextProperties::propertyOrDefault(KoSvgTextProperties::PropertyId id) const
{
    QVariant value = m_d->properties.value(id);
    if (value.isNull()) {
        value = defaultProperties().property(id);
    }
    return value;
}

QList<KoSvgTextProperties::PropertyId> KoSvgTextProperties::properties() const
{
    return m_d->properties.keys();
}

bool KoSvgTextProperties::isEmpty() const
{
    return m_d->properties.isEmpty();
}


bool KoSvgTextProperties::Private::isInheritable(PropertyId id) {
    return
            id != UnicodeBidiId &&
            id != DominantBaselineId &&
            id != AlignmentBaselineId &&
            id != BaselineShiftModeId &&
            id != BaselineShiftValueId;
}

void KoSvgTextProperties::resetNonInheritableToDefault()
{
    auto it = m_d->properties.begin();
    for (; it != m_d->properties.end(); ++it) {
        if (!m_d->isInheritable(it.key())) {
            it.value() = defaultProperties().property(it.key());
        }
    }
}

void KoSvgTextProperties::inheritFrom(const KoSvgTextProperties &parentProperties)
{
    auto it = parentProperties.m_d->properties.constBegin();
    for (; it != parentProperties.m_d->properties.constEnd(); ++it) {
        if (!hasProperty(it.key()) && m_d->isInheritable(it.key())) {
            setProperty(it.key(), it.value());
        }
    }
}

bool KoSvgTextProperties::inheritsProperty(KoSvgTextProperties::PropertyId id, const KoSvgTextProperties &parentProperties) const
{
    return !hasProperty(id) || parentProperties.property(id) == property(id);
}

KoSvgTextProperties KoSvgTextProperties::ownProperties(const KoSvgTextProperties &parentProperties) const
{
    KoSvgTextProperties result;

    auto it = m_d->properties.constBegin();
    for (; it != m_d->properties.constEnd(); ++it) {
        if (!parentProperties.hasProperty(it.key()) || parentProperties.property(it.key()) != it.value()) {
            result.setProperty(it.key(), it.value());
        }
    }

    return result;
}

inline qreal roundToStraightAngle(qreal value)
{
    return normalizeAngle(int((value + M_PI_4) / M_PI_2) * M_PI_2);
}

void KoSvgTextProperties::parseSvgTextAttribute(const SvgLoadingContext &context, const QString &command, const QString &value)
{
    if (command == "writing-mode") {
        setProperty(WritingModeId, KoSvgText::parseWritingMode(value));
    } else if (command == "glyph-orientation-vertical") {
        KoSvgText::AutoValue autoValue = KoSvgText::parseAutoValueAngular(value, context);

        if (!autoValue.isAuto) {
            autoValue.customValue = roundToStraightAngle(autoValue.customValue);
        }

        setProperty(GlyphOrientationVerticalId, KoSvgText::fromAutoValue(autoValue));
    } else if (command == "glyph-orientation-horizontal") {
        setProperty(GlyphOrientationHorizontalId, roundToStraightAngle(SvgUtil::parseUnitAngular(context.currentGC(), value)));
    } else if (command == "direction") {
        setProperty(DirectionId, KoSvgText::parseDirection(value));
    } else if (command == "unicode-bidi") {
        setProperty(UnicodeBidiId, KoSvgText::parseUnicodeBidi(value));
    } else if (command == "text-anchor") {
        setProperty(TextAnchorId, KoSvgText::parseTextAnchor(value));
    } else if (command == "dominant-baseline") {
        setProperty(DominantBaselineId, KoSvgText::parseDominantBaseline(value));
    } else if (command == "alignment-baseline") {
        setProperty(AlignmentBaselineId, KoSvgText::parseAlignmentBaseline(value));
    } else if (command == "baseline-shift") {
        KoSvgText::BaselineShiftMode mode = KoSvgText::parseBaselineShiftMode(value);
        setProperty(BaselineShiftModeId, mode);
        if (mode == KoSvgText::ShiftPercentage) {
            if (value.endsWith("%")) {
                setProperty(BaselineShiftValueId, SvgUtil::fromPercentage(value));
            } else {
                const qreal parsedValue = SvgUtil::parseUnitXY(context.currentGC(), value);
                const qreal lineHeight = propertyOrDefault(FontSizeId).toReal();

                if (lineHeight != 0.0) {
                    setProperty(BaselineShiftValueId, parsedValue / lineHeight);
                }
            }
        }
    } else if (command == "kerning") {
        setProperty(KerningId, KoSvgText::fromAutoValue(KoSvgText::parseAutoValueXY(value, context)));
    } else if (command == "letter-spacing") {
        setProperty(LetterSpacingId, KoSvgText::fromAutoValue(KoSvgText::parseAutoValueXY(value, context, "normal")));
    } else if (command == "word-spacing") {
        setProperty(WordSpacingId, KoSvgText::fromAutoValue(KoSvgText::parseAutoValueXY(value, context, "normal")));
    } else if (command == "font-family") {
        QStringList familiesList = value.split(',', QString::SkipEmptyParts);
        for (QString &family : familiesList) {
            family = family.trimmed();
            if ((family.startsWith('\"') && family.endsWith('\"')) ||
                (family.startsWith('\'') && family.endsWith('\''))) {

                family = family.mid(1, family.size() - 2);
            }
        }
        setProperty(FontFamiliesId, familiesList);

    } else if (command == "font-style") {
        const QFont::Style style =
            value == "italic" ? QFont::StyleItalic :
            value == "oblique" ? QFont::StyleOblique :
            QFont::StyleNormal;

        setProperty(FontStyleId, style);
    } else if (command == "font-variant") {
        const bool isSmallCaps = value == "small-caps";
        setProperty(FontIsSmallCapsId, isSmallCaps);

    } else if (command == "font-stretch") {
        int newStretch = 100;

        static const std::vector<int> fontStretches = {50, 62, 75, 87, 100, 112, 125, 150, 200};
        static const std::vector<QString> fontStretchNames =
            {"ultra-condensed","extra-condensed","condensed","semi-condensed",
             "normal",
             "semi-expanded","expanded","extra-expanded","ultra-expanded"};

        if (value == "wider") {
            auto it = std::upper_bound(fontStretches.begin(),
                                       fontStretches.end(),
                                       propertyOrDefault(KoSvgTextProperties::FontStretchId).toInt());

            newStretch = it != fontStretches.end() ? *it : fontStretches.back();
        } else if (value == "narrower") {
            auto it = std::upper_bound(fontStretches.rbegin(),
                                       fontStretches.rend(),
                                       propertyOrDefault(KoSvgTextProperties::FontStretchId).toInt(),
                                       std::greater<int>());

            newStretch = it != fontStretches.rend() ? *it : fontStretches.front();

        } else {
            auto it = std::find(fontStretchNames.begin(), fontStretchNames.end(), value);
            if (it != fontStretchNames.end()) {
                newStretch = fontStretches[it - fontStretchNames.begin()];
            }
        }

        setProperty(FontStretchId, newStretch);

    } else if (command == "font-weight") {
        int weight = QFont::Normal;

        // map svg weight to qt weight
        // svg value        qt value
        // 100,200,300      1, 17, 33
        // 400              50          (normal)
        // 500,600          58,66
        // 700              75          (bold)
        // 800,900          87,99

        static const std::vector<int> fontWeights = {1,17,33,50,58,66,75,87,99};
        static const std::vector<int> svgFontWeights = {100,200,300,400,500,600,700,800,900};

        if (value == "bold")
            weight = QFont::Bold;
        else if (value == "bolder") {
            auto it = std::upper_bound(fontWeights.begin(),
                                       fontWeights.end(),
                                       propertyOrDefault(FontWeightId).toInt());

            weight = it != fontWeights.end() ? *it : fontWeights.back();
        } else if (value == "lighter") {
            auto it = std::upper_bound(fontWeights.rbegin(),
                                       fontWeights.rend(),
                                       propertyOrDefault(FontWeightId).toInt(),
                                       std::greater<int>());

            weight = it != fontWeights.rend() ? *it : fontWeights.front();
        } else {
            bool ok = false;

            // try to read numerical weight value
            const int newWeight = value.toInt(&ok, 10);
            if (ok) {
                auto it = std::find(svgFontWeights.begin(), svgFontWeights.end(), newWeight);
                if (it != svgFontWeights.end()) {
                    weight = fontWeights[it - svgFontWeights.begin()];
                }
            }
        }

        setProperty(FontWeightId, weight);

    } else if (command == "font-size") {
        const qreal pointSize = SvgUtil::parseUnitY(context.currentGC(), value);
        if (pointSize > 0.0) {
            setProperty(FontSizeId, pointSize);
        }
    } else if (command == "font-size-adjust") {
        setProperty(FontSizeAdjustId, KoSvgText::fromAutoValue(KoSvgText::parseAutoValueY(value, context, "none")));

    } else if (command == "text-decoration") {
        using namespace KoSvgText;
        TextDecorations deco = DecorationNone;

        Q_FOREACH (const QString &param, value.split(' ', QString::SkipEmptyParts)) {
            if (param == "line-through") {
                deco |= DecorationLineThrough;
            } else if (param == "underline") {
                deco |= DecorationUnderline;
            } else if (param == "overline") {
                deco |= DecorationOverline;
            }
        }

        setProperty(TextDecorationId, QVariant::fromValue(deco));

    } else {
        qFatal("FATAL: Unknown SVG property: %s = %s", command.toUtf8().data(), value.toUtf8().data());
    }
}

QMap<QString, QString> KoSvgTextProperties::convertToSvgTextAttributes() const
{
    using namespace KoSvgText;

    QMap<QString, QString> result;

    if (hasProperty(WritingModeId)) {
        result.insert("writing-mode", writeWritingMode(WritingMode(property(WritingModeId).toInt())));
    }

    if (hasProperty(GlyphOrientationVerticalId)) {
        result.insert("glyph-orientation-vertical", writeAutoValue(property(GlyphOrientationVerticalId).value<AutoValue>()));
    }

    if (hasProperty(GlyphOrientationHorizontalId)) {
        result.insert("glyph-orientation-horizontal", writeAutoValue(property(GlyphOrientationHorizontalId).value<AutoValue>()));
    }

    if (hasProperty(DirectionId)) {
        result.insert("direction", writeDirection(Direction(property(DirectionId).toInt())));
    }

    if (hasProperty(UnicodeBidiId)) {
        result.insert("unicode-bidi", writeUnicodeBidi(UnicodeBidi(property(UnicodeBidiId).toInt())));
    }

    if (hasProperty(TextAnchorId)) {
        result.insert("text-anchor", writeTextAnchor(TextAnchor(property(TextAnchorId).toInt())));
    }

    if (hasProperty(DominantBaselineId)) {
        result.insert("dominant-baseline", writeDominantBaseline(DominantBaseline(property(DominantBaselineId).toInt())));
    }

    if (hasProperty(AlignmentBaselineId)) {
        result.insert("alignment-baseline", writeAlignmentBaseline(AlignmentBaseline(property(AlignmentBaselineId).toInt())));
    }

    if (hasProperty(BaselineShiftModeId)) {
        result.insert("baseline-shift", writeBaselineShiftMode(
                      BaselineShiftMode(property(BaselineShiftModeId).toInt()),
                      property(BaselineShiftValueId).toReal()));
    }

    if (hasProperty(KerningId)) {
        result.insert("kerning", writeAutoValue(property(KerningId).value<AutoValue>()));
    }

    if (hasProperty(LetterSpacingId)) {
        result.insert("letter-spacing", writeAutoValue(property(LetterSpacingId).value<AutoValue>(), "normal"));
    }

    if (hasProperty(WordSpacingId)) {
        result.insert("word-spacing", writeAutoValue(property(WordSpacingId).value<AutoValue>(), "normal"));
    }

    if (hasProperty(FontFamiliesId)) {
        result.insert("font-family", property(FontFamiliesId).toStringList().join(','));
    }

    if (hasProperty(FontStyleId)) {
        const QFont::Style style = QFont::Style(property(FontStyleId).toInt());

        const QString value =
            style == QFont::StyleItalic ? "italic" :
            style == QFont::StyleOblique ? "oblique" : "normal";

        result.insert("font-style", value);
    }

    if (hasProperty(FontIsSmallCapsId)) {
        result.insert("font-variant", property(FontIsSmallCapsId).toBool() ? "small-caps" : "normal");
    }

    if (hasProperty(FontStretchId)) {
        const int stretch = property(FontStretchId).toInt();

        static const std::vector<int> fontStretches = {50, 62, 75, 87, 100, 112, 125, 150, 200};
        static const std::vector<QString> fontStretchNames =
            {"ultra-condensed","extra-condensed","condensed","semi-condensed",
             "normal",
             "semi-expanded","expanded","extra-expanded","ultra-expanded"};

        auto it = std::lower_bound(fontStretches.begin(), fontStretches.end(), stretch);
        if (it != fontStretches.end()) {
            result.insert("font-stretch", fontStretchNames[it - fontStretches.begin()]);
        }
    }

    if (hasProperty(FontWeightId)) {
        const int weight = property(FontWeightId).toInt();

        static const std::vector<int> fontWeights = {1,17,33,50,58,66,75,87,99};
        static const std::vector<int> svgFontWeights = {100,200,300,400,500,600,700,800,900};

        auto it = std::lower_bound(fontWeights.begin(), fontWeights.end(), weight);
        if (it != fontWeights.end()) {
            result.insert("font-weight", KisDomUtils::toString(svgFontWeights[it - fontWeights.begin()]));
        }
    }

    if (hasProperty(FontSizeId)) {
        const qreal size = property(FontSizeId).toReal();
        result.insert("font-size", KisDomUtils::toString(size));
    }

    if (hasProperty(FontSizeAdjustId)) {
        result.insert("font-size-adjust", writeAutoValue(property(FontSizeAdjustId).value<AutoValue>()));
    }

    if (hasProperty(TextDecorationId)) {
        using namespace KoSvgText;
        TextDecorations deco = property(TextDecorationId).value<TextDecorations>();

        QStringList decoStrings;

        if (deco & DecorationUnderline) {
            decoStrings.append("underline");
        }

        if (deco & DecorationOverline) {
            decoStrings.append("overline");
        }

        if (deco & DecorationLineThrough) {
            decoStrings.append("line-through");
        }

        result.insert("text-decoration", decoStrings.join(' '));
    }

    return result;
}

QFont KoSvgTextProperties::generateFont() const
{
    QFont font;

    QStringList familiesList =
        propertyOrDefault(KoSvgTextProperties::FontFamiliesId).toStringList();

    if (!familiesList.isEmpty()) {
        font.setFamily(familiesList.first());
    }

    font.setPointSizeF(propertyOrDefault(KoSvgTextProperties::FontSizeId).toReal());

    const QFont::Style style =
        QFont::Style(propertyOrDefault(KoSvgTextProperties::FontStyleId).toInt());
    font.setStyle(style);

    font.setCapitalization(
        propertyOrDefault(KoSvgTextProperties::FontIsSmallCapsId).toBool() ?
            QFont::SmallCaps : QFont::MixedCase);

    font.setStretch(propertyOrDefault(KoSvgTextProperties::FontStretchId).toInt());

    font.setWeight(propertyOrDefault(KoSvgTextProperties::FontWeightId).toInt());

    using namespace KoSvgText;

    TextDecorations deco =
        propertyOrDefault(KoSvgTextProperties::TextDecorationId)
            .value<KoSvgText::TextDecorations>();

    font.setStrikeOut(deco & DecorationLineThrough);
    font.setUnderline(deco & DecorationUnderline);
    font.setOverline(deco & DecorationOverline);

    struct FakePaintDevice : public QPaintDevice
    {
        QPaintEngine *paintEngine() const override {
            return nullptr;
        }

        int metric(QPaintDevice::PaintDeviceMetric metric) const override {

            if (metric == QPaintDevice::PdmDpiX || metric == QPaintDevice::PdmDpiY) {
                return 72;
            }


            return QPaintDevice::metric(metric);
        }
    };

    // paint device is used only to initialize DPI, so
    // we can delete it right after creation of the font
    FakePaintDevice fake72DpiPaintDevice;
    return QFont(font, &fake72DpiPaintDevice);
}

QStringList KoSvgTextProperties::supportedXmlAttributes()
{
    QStringList attributes;
    attributes << "writing-mode" << "glyph-orientation-vertical" << "glyph-orientation-horizontal"
               << "direction" << "unicode-bidi" << "text-anchor"
               << "dominant-baseline" << "alignment-baseline" << "baseline-shift"
               << "kerning" << "letter-spacing" << "word-spacing";
    return attributes;
}

namespace {
Q_GLOBAL_STATIC(KoSvgTextProperties, s_defaultProperties)
}

const KoSvgTextProperties &KoSvgTextProperties::defaultProperties()
{
    if (!s_defaultProperties.exists()) {
        using namespace KoSvgText;

        s_defaultProperties->setProperty(WritingModeId, LeftToRight);
        s_defaultProperties->setProperty(DirectionId, DirectionLeftToRight);
        s_defaultProperties->setProperty(UnicodeBidiId, BidiNormal);
        s_defaultProperties->setProperty(TextAnchorId, AnchorStart);
        s_defaultProperties->setProperty(DominantBaselineId, DominantBaselineAuto);
        s_defaultProperties->setProperty(AlignmentBaselineId, AlignmentBaselineAuto);
        s_defaultProperties->setProperty(BaselineShiftModeId, ShiftNone);
        s_defaultProperties->setProperty(BaselineShiftValueId, 0.0);
        s_defaultProperties->setProperty(KerningId, fromAutoValue(AutoValue()));
        s_defaultProperties->setProperty(GlyphOrientationVerticalId, fromAutoValue(AutoValue()));
        s_defaultProperties->setProperty(GlyphOrientationHorizontalId, fromAutoValue(AutoValue()));
        s_defaultProperties->setProperty(LetterSpacingId, fromAutoValue(AutoValue()));
        s_defaultProperties->setProperty(WordSpacingId, fromAutoValue(AutoValue()));

        QFont font;

        s_defaultProperties->setProperty(FontFamiliesId, font.family());
        s_defaultProperties->setProperty(FontStyleId, font.style());
        s_defaultProperties->setProperty(FontIsSmallCapsId, bool(font.capitalization() == QFont::SmallCaps));
        s_defaultProperties->setProperty(FontStretchId, font.stretch());
        s_defaultProperties->setProperty(FontWeightId, font.weight());
        s_defaultProperties->setProperty(FontSizeId, font.pointSizeF());
        s_defaultProperties->setProperty(FontSizeAdjustId, fromAutoValue(AutoValue()));

        {
            using namespace KoSvgText;
            TextDecorations deco = DecorationNone;

            if (font.underline()) {
                deco |= DecorationUnderline;
            }

            if (font.strikeOut()) {
                deco |= DecorationLineThrough;
            }

            if (font.overline()) {
                deco |= DecorationOverline;
            }

            s_defaultProperties->setProperty(TextDecorationId, QVariant::fromValue(deco));
        }
    }
    return *s_defaultProperties;
}

