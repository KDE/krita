/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "CssQmlUnitConverter.h"
#include <QVariant>
#include <KoUnit.h>
#include <KoSvgText.h>
#include <KoSvgTextProperties.h>
#include <lager/KoSvgTextPropertiesModel.h>
#include <KLocalizedString>

struct CssQmlUnitConverter::Private {
    QMap<int, int> dataUnitMap;

    qreal dpi{72.0};
    qreal dataMultiplier{1.0};
    qreal dataValue{0.0};
    int dataUnit{0};
    int userUnit{0};
    qreal percentageReference;

    KoUnit absoluteUnitConverter;

    // fontmetrics
    KoCSSFontInfo fontInfo;
    int fontLineGap = 0;
    KoSvgText::FontMetrics metrics;
};

// Map of absolute values.
// by making this a map, it always stays order in the order of KoUnit::type.
const QMap<KoUnit::Type, CssQmlUnitConverter::UserUnits> koUnitMap = {
    {KoUnit::Millimeter, CssQmlUnitConverter::UserUnits::Mm},
    {KoUnit::Point, CssQmlUnitConverter::UserUnits::Pt},
    {KoUnit::Inch, CssQmlUnitConverter::UserUnits::Inch},
    {KoUnit::Centimeter, CssQmlUnitConverter::UserUnits::Cm},
    {KoUnit::Pixel, CssQmlUnitConverter::UserUnits::Px},
};

const qreal koUnitFactor = 1/72.0;

CssQmlUnitConverter::CssQmlUnitConverter(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->absoluteUnitConverter = KoUnit(KoUnit::Point, d->dpi);
    d->fontInfo = KoSvgTextProperties::defaultProperties().cssFontInfo();
    d->metrics = KoSvgTextProperties::defaultProperties().metrics(false);
}

CssQmlUnitConverter::~CssQmlUnitConverter()
{
}

void CssQmlUnitConverter::setDataUnitMap(const QVariantList &unitMap)
{
    QMap<int, int> map;
    Q_FOREACH(QVariant var, unitMap) {
        QVariantMap m = var.toMap();
        bool ok = false;
        const int user = m.value("user").toInt(&ok);
        const int data = m.value("data").toInt(&ok);
        if (!ok) {
            qWarning() << Q_FUNC_INFO << "unitMap has wrong format";
            return;
        }
        map.insert(user, data);
    }

    if (d->dataUnitMap != map) {
        d->dataUnitMap = map;
        emit userUnitModelChanged();
    }
}

void CssQmlUnitConverter::setFontMetricsFromTextPropertiesModel(KoSvgTextPropertiesModel *textPropertiesModel, bool isFontSize, bool isLineHeight)
{
    KoSvgTextProperties main;
    if (textPropertiesModel) {
        KoSvgTextPropertyData data = textPropertiesModel->textData.get();
        main = data.commonProperties;
        // remove fontsize or lineheight for those specific properties, so the calculation is normal.
        if (isFontSize) {
            main.removeProperty(KoSvgTextProperties::FontSizeId);
        } else if (isLineHeight) {
            main.removeProperty(KoSvgTextProperties::LineHeightId);
        }
        main.inheritFrom(data.inheritedProperties);
    }
    KoCSSFontInfo info = main.cssFontInfo();
    if (info.size < 0) return;
    if (d->fontInfo == info) {
        return;
    }
    d->fontInfo = main.cssFontInfo();
    d->metrics = main.metrics(false);
    d->fontLineGap = d->metrics.lineGap;
    d->metrics = main.applyLineHeight(d->metrics);
}

void CssQmlUnitConverter::setFromNormalLineHeight()
{
    const qreal multiplier = d->fontInfo.size / d->metrics.fontSize;
    qreal normalLineHeight = (d->metrics.ascender - d->metrics.descender + d->fontLineGap)*multiplier;
    setDataValue(convertToRelativeValue(normalLineHeight, UserUnits(d->dataUnitMap.key(d->dataUnit))));
}

void CssQmlUnitConverter::setDataValueAndUnit(const qreal value, const int unit)
{
    if (qFuzzyCompare(d->dataValue, value) && d->dataUnit == unit) {
        return;
    }
    d->dataUnit = unit;
    d->dataValue = value;
    d->userUnit = d->dataUnitMap.key(d->dataUnit);

    emit dataValueChanged();
    emit dataUnitChanged();
    emit userUnitChanged();
    emit userValueChanged();
}

qreal CssQmlUnitConverter::dpi() const
{
    return d->dpi;
}

void CssQmlUnitConverter::setDpi(qreal newDpi)
{
    if (qFuzzyCompare(d->dpi, newDpi))
        return;
    d->dpi = newDpi;
    d->absoluteUnitConverter.setFactor(newDpi * koUnitFactor);
    emit dpiChanged();
}

qreal CssQmlUnitConverter::dataMultiplier() const
{
    return d->dataMultiplier;
}

void CssQmlUnitConverter::setDataMultiplier(qreal newDataMultiplier)
{
    if (qFuzzyCompare(d->dataMultiplier, newDataMultiplier))
        return;
    d->dataMultiplier = newDataMultiplier;
    emit dataMultiplierChanged();
}

qreal CssQmlUnitConverter::dataValue() const
{
    return d->dataValue;
}

void CssQmlUnitConverter::setDataValue(qreal newDataValue)
{
    if (qFuzzyCompare(d->dataValue, newDataValue))
        return;
    d->dataValue = newDataValue;
    emit dataValueChanged();
    emit userValueChanged();
}

int CssQmlUnitConverter::dataUnit() const
{
    return d->dataUnit;
}

void CssQmlUnitConverter::setDataUnit(int newDataUnit)
{
    if (d->dataUnit == newDataUnit)
        return;
    if (d->dataUnitMap.key(d->dataUnit) == d->userUnit)
        return;
    setUserUnit(d->dataUnitMap.key(d->dataUnit));
}

qreal CssQmlUnitConverter::userValue() const
{
    const bool isAbsolute = koUnitMap.values().contains(UserUnits(d->userUnit));
    return (isAbsolute? d->absoluteUnitConverter.toUserValue(d->dataValue): d->dataValue) * d->dataMultiplier;
}

void CssQmlUnitConverter::setUserValue(qreal newUserValue)
{
    if (qFuzzyCompare(userValue(), newUserValue))
        return;
    const bool isAbsolute = koUnitMap.values().contains(UserUnits(d->userUnit));
    d->dataValue = (isAbsolute? d->absoluteUnitConverter.fromUserValue(newUserValue): newUserValue) / d->dataMultiplier;
    emit userValueChanged();
    emit dataValueChanged();
}

int CssQmlUnitConverter::userUnit() const
{
    return d->userUnit;
}

void CssQmlUnitConverter::setUserUnit(int newUserUnit)
{
    if (d->userUnit == newUserUnit)
        return;
    UserUnits newType = UserUnits(newUserUnit);
    const bool newIsAbsolute = koUnitMap.values().contains(newType);
    const bool oldIsAbsolute = koUnitMap.values().contains(UserUnits(d->userUnit));

    const qreal currentAbsoluteValue = oldIsAbsolute?
                d->absoluteUnitConverter.fromUserValue(d->dataValue):
                convertFromRelativeValue(d->dataValue, UserUnits(d->userUnit));

    if (newIsAbsolute && oldIsAbsolute) {
        KoUnit newUnitConverter(koUnitMap.key(newType), d->dpi * koUnitFactor);
        d->absoluteUnitConverter = newUnitConverter;
        emit userValueChanged();
    } else if (newIsAbsolute) {
        KoUnit newUnitConverter(koUnitMap.key(newType), d->dpi* koUnitFactor);
        d->absoluteUnitConverter = newUnitConverter;
        d->dataUnit = d->dataUnitMap.value(UserUnits::Pt);
        emit dataUnitChanged();
        setDataValue(currentAbsoluteValue);
    } else { // old value was absolute, and new value is relative, or both values are relative.
        d->dataUnit = d->dataUnitMap.value(newUserUnit);
        emit dataUnitChanged();
        setDataValue(convertToRelativeValue(currentAbsoluteValue, newType));
    }

    d->userUnit = newUserUnit;
    emit userUnitChanged();
}

QVariantList CssQmlUnitConverter::userUnitModel() const
{
    QVariantList userUnitModel;

    const QString descriptionKey = "description";
    const QString valueKey = "value";
    Q_FOREACH(const int key, d->dataUnitMap.keys()) {
        UserUnits keyUnit = UserUnits(key);

        if (keyUnit == Pt) {
            Q_FOREACH(KoUnit::Type koUnitKey, koUnitMap.keys()) {
                QVariantMap unit;
                unit.insert(descriptionKey, KoUnit::unitDescription(koUnitKey));
                unit.insert(valueKey, koUnitMap.value(koUnitKey));
                userUnitModel.append(unit);
            }
        } else {
            QVariantMap unit;
            if (keyUnit == Em) {
                unit.insert(descriptionKey, i18nc("@label:inlistbox", "Font Size (em)"));
            } else if (keyUnit == Ex) {
                unit.insert(descriptionKey, i18nc("@label:inlistbox", "X Height (ex)"));
            } else if (keyUnit == Cap) {
                unit.insert(descriptionKey, i18nc("@label:inlistbox", "Cap Height (cap)"));
            } else if (keyUnit == Ch) {
                unit.insert(descriptionKey, i18nc("@label:inlistbox", "Proportional Advance (ch)"));
            } else if (keyUnit == Ic) {
                unit.insert(descriptionKey, i18nc("@label:inlistbox", "Ideographic Advance (ic)"));
            } else if (keyUnit == Lh) {
                unit.insert(descriptionKey, i18nc("@label:inlistbox", "Line Height (lh)"));
            } else if (keyUnit == Spaces) {
                unit.insert(descriptionKey, i18nc("@label:inlistbox", "Spaces (Sp)"));
            } else if (keyUnit == Lines) {
                unit.insert(descriptionKey, i18nc("@label:inlistbox", "Lines (Ln)"));
            } else if (keyUnit == Percentage) {
                unit.insert(descriptionKey, i18nc("@label:inlistbox", "Percentage (%)"));
            }
            unit.insert(valueKey, keyUnit);
            userUnitModel.append(unit);
        }
    }

    return userUnitModel;
}

QString CssQmlUnitConverter::symbol() const
{
    UserUnits unit = UserUnits(d->userUnit);
    const bool isAbsolute = koUnitMap.values().contains(unit);
    if (isAbsolute) {
        return d->absoluteUnitConverter.symbol();
    }
    if (unit == Em) {
        return QString("em");
    } else if (unit == Ex) {
        return QString("ex");
    } else if (unit == Cap) {
        return QString("cap");
    } else if (unit == Ch) {
        return QString("ch");
    } else if (unit == Ic) {
        return QString("ic");
    } else if (unit == Lh) {
        return QString("lh");
    } else if (unit == Lines) {
        return QString("Ln");
    } else if (unit == Spaces) {
        return QString("Sp");
    } else if (unit == Percentage) {
        return QString("%");
    }
    return QString();
}

qreal CssQmlUnitConverter::percentageReference() const
{
    return d->percentageReference;
}

void CssQmlUnitConverter::setPercentageReference(qreal newPercentageReference)
{
    if (qFuzzyCompare(d->percentageReference, newPercentageReference))
        return;
    d->percentageReference = newPercentageReference;
    emit percentageReferenceChanged();
}

qreal metricsMultiplier(const CssQmlUnitConverter::UserUnits type, const KoSvgText::FontMetrics metrics, const qreal fontSize, const qreal percentageReference) {
    const qreal multiplier = fontSize / metrics.fontSize;
    if (type == CssQmlUnitConverter::Em) {
        return fontSize;
    } else if (type == CssQmlUnitConverter::Ex) {
        return metrics.xHeight * multiplier;
    } else if (type == CssQmlUnitConverter::Cap) {
        return metrics.capHeight * multiplier;
    } else if (type == CssQmlUnitConverter::Ch) {
        return metrics.zeroAdvance * multiplier;
    } else if (type == CssQmlUnitConverter::Ic) {
        return metrics.ideographicAdvance * multiplier;
    } else if (type == CssQmlUnitConverter::Lh) {
        return (metrics.ascender - metrics.descender + metrics.lineGap) * multiplier;
    } else if (type == CssQmlUnitConverter::Lines) {
        return (metrics.ascender - metrics.descender) * multiplier;
    } else if (type == CssQmlUnitConverter::Spaces) {
        return metrics.spaceAdvance * multiplier;
    } else if (type == CssQmlUnitConverter::Percentage) {
        return percentageReference;
    }
    return 1.0;
}

qreal CssQmlUnitConverter::convertToRelativeValue(const qreal value, const UserUnits type) const
{
    return value / metricsMultiplier(type, d->metrics, d->fontInfo.size, d->percentageReference);
}

qreal CssQmlUnitConverter::convertFromRelativeValue(const qreal value, const UserUnits type) const
{
    return value * metricsMultiplier(type, d->metrics, d->fontInfo.size, d->percentageReference);
}
