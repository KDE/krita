/*
   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KIS_HAIRY_INK_OPTION_H
#define KIS_HAIRY_INK_OPTION_H

#include <kis_paintop_option.h>

const QString HAIRY_INK_DEPLETION_ENABLED = "HairyInk/enabled";
const QString HAIRY_INK_AMOUNT = "HairyInk/inkAmount";
const QString HAIRY_INK_USE_SATURATION = "HairyInk/useSaturation";
const QString HAIRY_INK_USE_OPACITY = "HairyInk/useOpacity";
const QString HAIRY_INK_USE_WEIGHTS = "HairyInk/useWeights";
const QString HAIRY_INK_PRESSURE_WEIGHT = "HairyInk/pressureWeights";
const QString HAIRY_INK_BRISTLE_LENGTH_WEIGHT = "HairyInk/bristleLengthWeights";
const QString HAIRY_INK_BRISTLE_INK_AMOUNT_WEIGHT = "HairyInk/bristleInkAmountWeight";
const QString HAIRY_INK_DEPLETION_WEIGHT = "HairyInk/inkDepletionWeight";
const QString HAIRY_INK_DEPLETION_CURVE = "HairyInk/inkDepletionCurve";
const QString HAIRY_INK_SOAK = "HairyInk/soak";

class KisInkOptionsWidget;

class KisHairyInkOption : public KisPaintOpOption
{
public:
    KisHairyInkOption();
    ~KisHairyInkOption() override;

    int inkAmount() const;
    QList<float> curve() const;

    bool useSaturation() const;
    bool useOpacity() const;
    bool useWeights() const;

    int pressureWeight() const;
    int bristleLengthWeight() const;
    int bristleInkAmountWeight() const;
    int inkDepletionWeight() const;

    int m_curveSamples;

    void writeOptionSetting(KisPropertiesConfigurationSP config) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP config) override;
private:
    KisInkOptionsWidget * m_options;
};

#endif // KIS_HAIRY_SHAPE_OPTION_H

