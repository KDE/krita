/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisStandardOptionData.h"

#include <KisPaintOpOptionWidgetUtils.h>
#include <KisPaintopSettingsIds.h>


namespace KisPaintOpOptionWidgetUtils {

namespace detail {
KisCurveOptionWidget2 *createOpacityOptionWidgetImpl(bool isCheckable, KisPaintOpOption::PaintopCategory category, const QString &prefix)
{
    return createCurveOptionWidget(KisOpacityOptionData(isCheckable, prefix),
                                   category,
                                   i18n("Transparent"),
                                   i18n("Opaque"));
}
KisCurveOptionWidget2 *createRotationOptionWidgetImpl(KisPaintOpOption::PaintopCategory category, const QString &prefix)
{
    return createCurveOptionWidget(KisRotationOptionData(prefix),
                                   category,
                                   i18n("-180°"),
                                   i18n("180°"));
}
}

KisCurveOptionWidget2 *createOpacityOptionWidget()
{
    return detail::createOpacityOptionWidgetImpl(false, KisPaintOpOption::GENERAL, "");
}

KisCurveOptionWidget2 *createFlowOptionWidget()
{
    return createCurveOptionWidget(KisFlowOptionData(),
                                   KisPaintOpOption::GENERAL);
}

KisCurveOptionWidget2 *createRatioOptionWidget()
{
    return createCurveOptionWidget(KisRatioOptionData(),
                                   KisPaintOpOption::GENERAL);
}

KisCurveOptionWidget2 *createSoftnessOptionWidget()
{
    return createCurveOptionWidget(KisSoftnessOptionData(),
                                   KisPaintOpOption::GENERAL,
                                   i18n("Soft"),
                                   i18n("Hard"));
}

KisCurveOptionWidget2 *createRotationOptionWidget()
{
    return detail::createRotationOptionWidgetImpl(KisPaintOpOption::GENERAL, "");
}

KisCurveOptionWidget2 *createDarkenOptionWidget()
{
    return createCurveOptionWidget(KisDarkenOptionData(),
                                   KisPaintOpOption::COLOR,
                                   i18n("0.0"),
                                   i18n("1.0"));
}

KisCurveOptionWidget2 *createMixOptionWidget()
{
    return createCurveOptionWidget(KisMixOptionData(),
                                   KisPaintOpOption::COLOR,
                                   i18nc("Background painting color", "Background"),
                                   i18nc("Foreground painting color", "Foreground"));
}

namespace detail {
QString hueMinLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(0° is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+180°");
    QString zeroPercent = i18n("-180°");

    return QString(zeroPercent + br + i18n("CCW hue") + br + activeColorMsg);
}

QString hueMaxLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(0° is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+180°");
    QString zeroPercent = i18n("-180°");

    return QString(fullPercent + br + i18n("CW hue"));
}

QString saturationMinLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(50% is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+100%");
    QString zeroPercent = i18n("-100%");

    return QString(zeroPercent + br + i18n("Less saturation ") + br + activeColorMsg);

}

QString saturationMaxLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(50% is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+100%");
    QString zeroPercent = i18n("-100%");

    return QString(fullPercent + br + i18n("More saturation"));
}

QString valueMinLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(50% is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+100%");
    QString zeroPercent = i18n("-100%");

    return QString(zeroPercent + br + i18nc("Lower HSV brightness", "Lower value ") + br + activeColorMsg);

}

QString valueMaxLabel()
{
    // xgettext: no-c-format
    QString activeColorMsg = i18n("(50% is active color)");
    QString br("<br />");
    QString fullPercent = i18n("+100%");
    QString zeroPercent = i18n("-100%");

    return QString(fullPercent + br + i18nc("Higher HSV brightness", "Higher value"));


}
}

KisCurveOptionWidget2 *createHueOptionWidget()
{
    return createCurveOptionWidget(KisHueOptionData(), KisPaintOpOption::COLOR,
                                   detail::hueMinLabel(),
                                   detail::hueMaxLabel(),
                                   -180, 180, i18n("°"));
}

KisCurveOptionWidget2 *createSaturationOptionWidget()
{
    return createCurveOptionWidget(KisSaturationOptionData(), KisPaintOpOption::COLOR,
                                   detail::saturationMinLabel(),
                                   detail::saturationMaxLabel());
}

KisCurveOptionWidget2 *createValueOptionWidget()
{
    return createCurveOptionWidget(KisValueOptionData(),
                                   KisPaintOpOption::COLOR,
                                   detail::valueMinLabel(),
                                   detail::valueMaxLabel());
}

KisCurveOptionWidget2 *createRateOptionWidget()
{
    return createCurveOptionWidget(KisRateOptionData(),
                                   KisPaintOpOption::COLOR);
}

KisCurveOptionWidget2 *createStrengthOptionWidget()
{
    return createCurveOptionWidget(KisStrengthOptionData(),
                                   KisPaintOpOption::TEXTURE);
}

KisCurveOptionWidget2 *createMaskingOpacityOptionWidget()
{
    return detail::createOpacityOptionWidgetImpl(true, KisPaintOpOption::MASKING_BRUSH, KisPaintOpUtils::MaskingBrushPresetPrefix);
}

KisCurveOptionWidget2 *createMaskingFlowOptionWidget()
{
    return createCurveOptionWidget(KisFlowOptionData(true, KisPaintOpUtils::MaskingBrushPresetPrefix),
                                   KisPaintOpOption::MASKING_BRUSH);
}

KisCurveOptionWidget2 *createMaskingRatioOptionWidget()
{
    return createCurveOptionWidget(KisRatioOptionData(KisPaintOpUtils::MaskingBrushPresetPrefix),
                                   KisPaintOpOption::MASKING_BRUSH);
}

KisCurveOptionWidget2 *createMaskingRotationOptionWidget()
{
    return detail::createRotationOptionWidgetImpl(KisPaintOpOption::MASKING_BRUSH, KisPaintOpUtils::MaskingBrushPresetPrefix);
}

}
