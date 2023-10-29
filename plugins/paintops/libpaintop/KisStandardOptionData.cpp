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
KisCurveOptionWidget *createOpacityOptionWidgetImpl(KisPaintOpOption::PaintopCategory category, const QString &prefix)
{
    return createCurveOptionWidget(KisOpacityOptionData(prefix),
                                   category,
                                   i18n("Transparent"),
                                   i18n("Opaque"));
}
KisCurveOptionWidget *createRotationOptionWidgetImpl(KisPaintOpOption::PaintopCategory category, const QString &prefix)
{
    return createCurveOptionWidget(KisRotationOptionData(prefix),
                                   category,
                                   i18n("-180°"),
                                   i18n("180°"));
}
}

KisCurveOptionWidget *createOpacityOptionWidget()
{
    return detail::createOpacityOptionWidgetImpl(KisPaintOpOption::GENERAL, "");
}

KisCurveOptionWidget *createFlowOptionWidget()
{
    return createCurveOptionWidget(KisFlowOptionData(),
                                   KisPaintOpOption::GENERAL);
}

KisCurveOptionWidget *createRatioOptionWidget()
{
    return createCurveOptionWidget(KisRatioOptionData(),
                                   KisPaintOpOption::GENERAL);
}

KisCurveOptionWidget *createSoftnessOptionWidget()
{
    return createCurveOptionWidget(KisSoftnessOptionData(),
                                   KisPaintOpOption::GENERAL,
                                   i18n("Soft"),
                                   i18n("Hard"));
}

KisCurveOptionWidget *createRotationOptionWidget()
{
    return detail::createRotationOptionWidgetImpl(KisPaintOpOption::GENERAL, "");
}

KisCurveOptionWidget *createDarkenOptionWidget()
{
    return createCurveOptionWidget(KisDarkenOptionData(),
                                   KisPaintOpOption::COLOR,
                                   i18n("0.0"),
                                   i18n("1.0"));
}

KisCurveOptionWidget *createMixOptionWidget()
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

KisCurveOptionWidget *createHueOptionWidget()
{
    return createCurveOptionWidget(KisHueOptionData(), KisPaintOpOption::COLOR,
                                   detail::hueMinLabel(),
                                   detail::hueMaxLabel(),
                                   -180, 180, i18n("°"));
}

KisCurveOptionWidget *createSaturationOptionWidget()
{
    return createCurveOptionWidget(KisSaturationOptionData(), KisPaintOpOption::COLOR,
                                   detail::saturationMinLabel(),
                                   detail::saturationMaxLabel());
}

KisCurveOptionWidget *createValueOptionWidget()
{
    return createCurveOptionWidget(KisValueOptionData(),
                                   KisPaintOpOption::COLOR,
                                   detail::valueMinLabel(),
                                   detail::valueMaxLabel());
}

KisCurveOptionWidget *createRateOptionWidget()
{
    return createCurveOptionWidget(KisRateOptionData(),
                                   KisPaintOpOption::COLOR);
}

KisCurveOptionWidget *createStrengthOptionWidget()
{
    return createCurveOptionWidget(KisStrengthOptionData(),
                                   KisPaintOpOption::TEXTURE);
}

KisCurveOptionWidget *createMaskingOpacityOptionWidget()
{
    return detail::createOpacityOptionWidgetImpl(KisPaintOpOption::MASKING_BRUSH, KisPaintOpUtils::MaskingBrushPresetPrefix);
}

KisCurveOptionWidget *createMaskingFlowOptionWidget()
{
    return createCurveOptionWidget(KisFlowOptionData(KisPaintOpUtils::MaskingBrushPresetPrefix),
                                   KisPaintOpOption::MASKING_BRUSH);
}

KisCurveOptionWidget *createMaskingRatioOptionWidget()
{
    return createCurveOptionWidget(KisRatioOptionData(KisPaintOpUtils::MaskingBrushPresetPrefix),
                                   KisPaintOpOption::MASKING_BRUSH);
}

KisCurveOptionWidget *createMaskingRotationOptionWidget()
{
    return detail::createRotationOptionWidgetImpl(KisPaintOpOption::MASKING_BRUSH, KisPaintOpUtils::MaskingBrushPresetPrefix);
}

}
