/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIS_ROUND_MARKER_OPTION_H
#define KIS_ROUND_MARKER_OPTION_H

#include <kis_paintop_option.h>

class KisRoundMarkerOptionWidget;

class KisRoundMarkerOption : public KisPaintOpOption
{
public:
    KisRoundMarkerOption();
    ~KisRoundMarkerOption();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const;
    void readOptionSetting(KisPropertiesConfigurationSP setting);

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
