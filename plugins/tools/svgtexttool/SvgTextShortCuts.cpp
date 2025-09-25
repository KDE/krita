/*
 *  SPDX-FileCopyrightText: 2025 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "SvgTextShortCuts.h"
#include <QAction>
#include <KoSvgTextProperties.h>

#include <KisStaticInitializer.h>

/**
 * @brief The SvgTextShortcutInfo class
 * This
 */
struct SvgTextShortcutInfo : public boost::equality_comparable<SvgTextShortcutInfo> {

    enum ActionType {
        Set, ///< Will set value1, cannot be toggled.
        Toggle, ///< Toggle will test "testValue", and toggle between value1 and value 2;
        Increase, //< Will increase by value 1.
        Decrease //< Will decrease by value 1.
    };
    SvgTextShortcutInfo() {}

    static SvgTextShortcutInfo propertyToggle(KoSvgTextProperties::PropertyId _propertyId,
                                              QVariant _value1, QVariant _value2, QVariant _testValue) {
        SvgTextShortcutInfo info;
        info.propertyId = _propertyId;
        info.type = Toggle;
        info.value1 = _value1;
        info.value2 = _value2;
        info.testValue = _testValue;
        return info;
    }

    static SvgTextShortcutInfo propertyChange(KoSvgTextProperties::PropertyId _propertyId, QVariant _value1, bool _increase) {
        SvgTextShortcutInfo info;
        info.propertyId = _propertyId;
        info.type = _increase? Increase: Decrease;
        info.value1 = _value1;
        return info;
    }

    static SvgTextShortcutInfo propertySet(KoSvgTextProperties::PropertyId _propertyId, QVariant _value1) {
        SvgTextShortcutInfo info;
        info.propertyId = _propertyId;
        info.type = Set;
        info.value1 = _value1;
        return info;
    }

    KoSvgTextProperties::PropertyId propertyId;
    ActionType type;
    QVariant value1;
    QVariant value2;
    QVariant testValue;



    bool operator==(const SvgTextShortcutInfo & other) const {
        return (propertyId == other.propertyId
                && type == other.type
                && value1 == other.value1
                && value2 == other.value2
                && testValue == other.testValue);
    }
};

Q_DECLARE_METATYPE(SvgTextShortcutInfo)

KIS_DECLARE_STATIC_INITIALIZER {
    qRegisterMetaType<SvgTextShortcutInfo>("SvgTextShortcutInfo");
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QMetaType::registerEqualsComparator<SvgTextShortcutInfo>();
#endif
}

const QMap<QString, SvgTextShortcutInfo> textShortCuts = {
    {
        "svg_weight_bold",
        SvgTextShortcutInfo::propertyToggle(KoSvgTextProperties::FontWeightId,
        QVariant(400), QVariant(700), QVariant(500))
    },

    {
        "svg_weight_normal",
        SvgTextShortcutInfo::propertySet(KoSvgTextProperties::FontWeightId,
        QVariant(400))
    },
    {
        "svg_weight_demi",
        SvgTextShortcutInfo::propertySet(KoSvgTextProperties::FontWeightId,
        QVariant(600))
    },
    {
        "svg_weight_black",
        SvgTextShortcutInfo::propertySet(KoSvgTextProperties::FontWeightId,
        QVariant(900))
    },
    {
        "svg_weight_light",
        SvgTextShortcutInfo::propertySet(KoSvgTextProperties::FontWeightId,
        QVariant(300))
    },

    {
        "svg_format_italic",
        SvgTextShortcutInfo::propertyToggle(KoSvgTextProperties::FontStyleId,
        QVariant::fromValue(KoSvgText::CssFontStyleData(QFont::StyleNormal)),
        QVariant::fromValue(KoSvgText::CssFontStyleData(QFont::StyleItalic)),
        QVariant::fromValue(KoSvgText::CssFontStyleData(QFont::StyleNormal)))
    },

    {
        "svg_increase_font_size",
        SvgTextShortcutInfo::propertyChange(KoSvgTextProperties::FontSizeId,
        QVariant::fromValue(1), true)
    },

    {
        "svg_decrease_font_size",
        SvgTextShortcutInfo::propertyChange(KoSvgTextProperties::FontSizeId,
        QVariant::fromValue(1), false)
    },

    {
        "svg_format_underline",
        SvgTextShortcutInfo::propertyToggle(KoSvgTextProperties::TextDecorationLineId,
        QVariant(true), QVariant(false), QVariant(KoSvgText::DecorationUnderline))
    },
    {
        "svg_format_strike_through",
        SvgTextShortcutInfo::propertyToggle(KoSvgTextProperties::TextDecorationLineId,
        QVariant(true), QVariant(false), QVariant(KoSvgText::DecorationLineThrough))
    },
    {
        "svg_font_kerning",
        SvgTextShortcutInfo::propertyToggle(KoSvgTextProperties::KerningId,
        QVariant(true), QVariant(false), QVariant(true))
    },
    {
        "svg_align_right",
        SvgTextShortcutInfo::propertySet(KoSvgTextProperties::TextAlignAllId,
        QVariant(KoSvgText::AlignStart))
    },
    {
        "svg_align_left",
        SvgTextShortcutInfo::propertySet(KoSvgTextProperties::TextAlignAllId,
        QVariant(KoSvgText::AlignEnd))
    },
    {
        "svg_align_center",
        SvgTextShortcutInfo::propertySet(KoSvgTextProperties::TextAlignAllId,
        QVariant(KoSvgText::AlignCenter))
    },
    {
        "svg_align_justified",
        SvgTextShortcutInfo::propertySet(KoSvgTextProperties::TextAlignAllId,
        QVariant(KoSvgText::AlignJustify))
    },
    {
        "svg_format_subscript",
        SvgTextShortcutInfo::propertySet(KoSvgTextProperties::BaselineShiftModeId,
        QVariant(KoSvgText::ShiftSub))
    },
    {
        "svg_format_superscript",
        SvgTextShortcutInfo::propertySet(KoSvgTextProperties::BaselineShiftModeId,
        QVariant(KoSvgText::ShiftSuper))
    }
};

QStringList SvgTextShortCuts::possibleActions()
{
    return textShortCuts.keys();
}

bool SvgTextShortCuts::configureAction(QAction *action, const QString &name)
{
    if (!textShortCuts.contains(name)) return false;
    if (!action) return false;
    SvgTextShortcutInfo info = textShortCuts.value(name);
    action->setData(QVariant::fromValue(info));
    return true;
}

bool SvgTextShortCuts::actionEnabled(QAction *action, const QList<KoSvgTextProperties> currentProperties) {
    if (!action || !action->isCheckable() || !action->data().canConvert<SvgTextShortcutInfo>()) return false;
    SvgTextShortcutInfo info = action->data().value<SvgTextShortcutInfo>();

    if (info.type != SvgTextShortcutInfo::Toggle && info.type != SvgTextShortcutInfo::Set) {
        return false;
    }
    const QVariant testValue = info.type == SvgTextShortcutInfo::Toggle? info.testValue: info.value1;

    for (auto properties = currentProperties.begin(); properties != currentProperties.end(); properties++) {
        const QVariant oldValue = properties->propertyOrDefault(info.propertyId);


        if (oldValue.canConvert<KoSvgText::TextDecorations>() && info.propertyId == KoSvgTextProperties::TextDecorationLineId) {

            const KoSvgText::TextDecorations oldDecor = oldValue.value<KoSvgText::TextDecorations>();
            return oldDecor.testFlag(KoSvgText::TextDecoration(testValue.toInt()));

        } else if (oldValue.canConvert<KoSvgText::CssFontStyleData>()) {
            const KoSvgText::CssFontStyleData testVal = testValue.value<KoSvgText::CssFontStyleData>();
            const KoSvgText::CssFontStyleData currentVal = oldValue.value<KoSvgText::CssFontStyleData>();
            return (testVal.style == currentVal.style);
        } else if (oldValue.canConvert<KoSvgText::AutoValue>()) {
            const KoSvgText::AutoValue currentVal = oldValue.value<KoSvgText::AutoValue>();
            if (testValue.canConvert<KoSvgText::AutoValue>()) {
                return (testValue == oldValue);
            } else if (testValue.canConvert<double>()) {
                return currentVal.customValue == testValue.toDouble();
            } else {
                return currentVal.isAuto == testValue.toBool();
            }
        }  else if (oldValue.canConvert<KoSvgText::CssLengthPercentage>()) {
            const KoSvgText::CssLengthPercentage currentVal = oldValue.value<KoSvgText::CssLengthPercentage>();
            if (testValue.canConvert<KoSvgText::CssLengthPercentage>()) {
                return (testValue == oldValue);
            } else {
                return currentVal.value == testValue.toDouble();
            }
        } else if (oldValue.canConvert<KoSvgText::AutoLengthPercentage>()) {
            const KoSvgText::AutoLengthPercentage currentVal = oldValue.value<KoSvgText::CssLengthPercentage>();
            if (testValue.canConvert<KoSvgText::AutoLengthPercentage>()) {
                return (testValue == oldValue);
            } else if (testValue.canConvert<KoSvgText::CssLengthPercentage>()) {
                return (testValue == QVariant::fromValue(currentVal.length));
            } else {
                return currentVal.length.value == testValue.toDouble();
            }
        } else if (oldValue.canConvert<KoSvgText::LineHeightInfo>()) {
            const KoSvgText::LineHeightInfo currentVal = oldValue.value<KoSvgText::LineHeightInfo>();
            if (testValue.canConvert<KoSvgText::LineHeightInfo>()) {
                return (testValue == oldValue);
            } else if (testValue.canConvert<KoSvgText::CssLengthPercentage>()) {
                return (testValue == QVariant::fromValue(currentVal.length));
            } else {
                return currentVal.length.value == testValue.toDouble();
            }
        }
        return (testValue == oldValue);
    }
    return false;
}

/**
 * @brief toggleProperty
 * Handles toggling properties for getModifiedProperties
 * split out to make code easier to navigate.
 */
QVariant toggleProperty(SvgTextShortcutInfo info, QList<KoSvgTextProperties> currentProperties) {
    QVariant newVal;
    for(auto properties = currentProperties.begin(); properties != currentProperties.end(); properties++) {
        QVariant oldValue = properties->propertyOrDefault(info.propertyId);

        if (oldValue.canConvert<KoSvgText::TextDecorations>() && info.propertyId == KoSvgTextProperties::TextDecorationLineId) {
            KoSvgText::TextDecoration decor = KoSvgText::TextDecoration(info.testValue.toInt());
            KoSvgText::TextDecorations oldDecor = oldValue.value<KoSvgText::TextDecorations>();
            KoSvgText::TextDecorations newDecor;
            newDecor.setFlag(decor, !oldDecor.testFlag(decor));
            newVal = QVariant::fromValue(newDecor);
            if (oldDecor.testFlag(decor)) {
                break;
            }
        } else if (oldValue.canConvert<KoSvgText::CssFontStyleData>() && info.propertyId == KoSvgTextProperties::FontStyleId) {
            KoSvgText::CssFontStyleData testVal = info.testValue.value<KoSvgText::CssFontStyleData>();
            KoSvgText::CssFontStyleData currentVal = oldValue.value<KoSvgText::CssFontStyleData>();
            if (currentVal.style == testVal.style) {
                newVal = info.value2;
                break;
            } else {
                newVal = info.value1;
            }
        } else if (oldValue.canConvert<KoSvgText::AutoValue>() && info.propertyId == KoSvgTextProperties::KerningId) {
            bool testVal = info.testValue.toBool();
            KoSvgText::AutoValue currentVal = oldValue.value<KoSvgText::AutoValue>();
            if (currentVal.isAuto == testVal) {
                currentVal.customValue = 0.0;
                currentVal.isAuto = info.value2.toBool();
                newVal = QVariant::fromValue(currentVal);
                break;
            } else {
                currentVal.isAuto = info.value1.toBool();
                newVal = QVariant::fromValue(currentVal);
            }
        } else if (oldValue.canConvert<double>()) {
            if (oldValue.toDouble() > info.testValue.toDouble()) {
                newVal = info.value1;
                break;
            } else {
                newVal = info.value2;
            }
        } else if (oldValue.canConvert<int>()) {
            if (oldValue.toInt() > info.testValue.toInt()) {
                newVal = info.value1;
                break;
            } else {
                newVal = info.value2;
            }
        }
    }
    return newVal;
}

/**
 * @brief adjustValue
 * Handles increase/decrease value for getModifiedProperties,
 * split out to make code easier to navigate.
 *
 * TODO: handle max/min.
 */
QVariant adjustValue(SvgTextShortcutInfo info, QVariant oldValue) {
    QVariant newVal;

    if (oldValue.canConvert<KoSvgText::CssLengthPercentage>()) {
        KoSvgText::CssLengthPercentage length = oldValue.value<KoSvgText::CssLengthPercentage>();
        if (info.type == SvgTextShortcutInfo::Increase) {
            length.value += info.value1.toDouble();
        } else {
            length.value -= info.value1.toDouble();
        }
        newVal = QVariant::fromValue(length);
    } else if (oldValue.canConvert<KoSvgText::AutoLengthPercentage>()) {
        KoSvgText::AutoLengthPercentage length = oldValue.value<KoSvgText::AutoLengthPercentage>();
        length.isAuto = false;
        if (info.type == SvgTextShortcutInfo::Increase) {
            length.length.value += info.value1.toDouble();
        } else {
            length.length.value -= info.value1.toDouble();
        }
        newVal = QVariant::fromValue(length);
    } else if (oldValue.canConvert<KoSvgText::AutoValue>()) {
        KoSvgText::AutoValue value = oldValue.value<KoSvgText::AutoValue>();
        value.isAuto = false;
        if (info.type == SvgTextShortcutInfo::Increase) {
            value.customValue += info.value1.toDouble();
        } else {
            value.customValue -= info.value1.toDouble();
        }
        newVal = QVariant::fromValue(value);
    } else if (oldValue.canConvert<double>()) {
        double value = oldValue.toDouble();
        if (info.type == SvgTextShortcutInfo::Increase) {
            value += info.value1.toDouble();
        } else {
            value -= info.value1.toDouble();
        }
        newVal = QVariant::fromValue(value);
    } else if (oldValue.canConvert<int>()) {
        int value = oldValue.toInt();
        if (info.type == SvgTextShortcutInfo::Increase) {
            value += info.value1.toInt();
        } else {
            value -= info.value1.toInt();
        }
        newVal = QVariant::fromValue(value);
    }

    return newVal;
}

KoSvgTextProperties SvgTextShortCuts::getModifiedProperties(const QAction *action, QList<KoSvgTextProperties> currentProperties)
{
    if (!action || !action->data().canConvert<SvgTextShortcutInfo>() || currentProperties.isEmpty()) return KoSvgTextProperties();
    SvgTextShortcutInfo info = action->data().value<SvgTextShortcutInfo>();

    QVariant newVal;
    if (info.type == SvgTextShortcutInfo::Toggle) {
        newVal = toggleProperty(info, currentProperties);
    } else if (info.type == SvgTextShortcutInfo::Set) {
        KoSvgTextProperties properties = currentProperties.first();
        QVariant oldValue = properties.propertyOrDefault(info.propertyId);

        if (oldValue.canConvert<int>()) {
            newVal = info.value1;
        }
    } else {
        KoSvgTextProperties properties = currentProperties.first();
        QVariant oldValue = properties.propertyOrDefault(info.propertyId);

        newVal = adjustValue(info, oldValue);
    }
    KoSvgTextProperties newProperties;
    newProperties.setProperty(info.propertyId, newVal);
    return newProperties;
}
