/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_ROUND_MARKER_OPTION_H
#define KIS_ROUND_MARKER_OPTION_H

#include <kis_paintop_option.h>

class KisRoundMarkerOptionWidget;

class KisRoundMarkerOption : public KisPaintOpOption
{
public:
    KisRoundMarkerOption();
    ~KisRoundMarkerOption() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(KisPropertiesConfigurationSP setting) override;

private:
    KisRoundMarkerOptionWidget * m_options;

};

class RoundMarkerOption
{
public:
    qreal diameter;
    qreal spacing;
    bool use_auto_spacing;
    qreal auto_spacing_coeff;

    void readOptionSetting(const KisPropertiesConfiguration &config) {
        diameter = config.getDouble("diameter", 30.0);
        spacing = config.getDouble("spacing", 0.02);
        use_auto_spacing = config.getBool("useAutoSpacing", false);
        auto_spacing_coeff = config.getDouble("autoSpacingCoeff", 1.0);
    }

    void writeOptionSetting(KisPropertiesConfigurationSP config) const {
        config->setProperty("diameter", diameter);
        config->setProperty("spacing", spacing);
        config->setProperty("useAutoSpacing", use_auto_spacing);
        config->setProperty("autoSpacingCoeff", auto_spacing_coeff);
    }
};

#endif
