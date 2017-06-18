/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef KIS_HATCHING_OPTIONS_H
#define KIS_HATCHING_OPTIONS_H

#include <kis_paintop_option.h>

class KisPaintopLodLimitations;
class KisHatchingOptionsWidget;

class KisHatchingOptions : public KisPaintOpOption
{

public:
    KisHatchingOptions();
    ~KisHatchingOptions() override;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;
    void lodLimitations(KisPaintopLodLimitations *l) const override;

private:
    KisHatchingOptionsWidget *m_options;

};

struct HatchingOption {
    qreal angle;
    qreal separation;
    qreal thickness;
    qreal origin_x;
    qreal origin_y;

    bool bool_nocrosshatching;
    bool bool_perpendicular;
    bool bool_minusthenplus;
    bool bool_plusthenminus;
    bool bool_moirepattern;

    int separationintervals;


    void writeOptionSetting(KisPropertiesConfigurationSP setting) const {
        setting->setProperty("Hatching/angle", angle);
        setting->setProperty("Hatching/separation", separation);
        setting->setProperty("Hatching/thickness", thickness);
        setting->setProperty("Hatching/origin_x", origin_x);
        setting->setProperty("Hatching/origin_y", origin_y);

        setting->setProperty("Hatching/bool_nocrosshatching", bool_nocrosshatching);
        setting->setProperty("Hatching/bool_perpendicular", bool_perpendicular);
        setting->setProperty("Hatching/bool_minusthenplus", bool_minusthenplus);
        setting->setProperty("Hatching/bool_plusthenminus", bool_plusthenminus);
        setting->setProperty("Hatching/bool_moirepattern", bool_moirepattern);

        setting->setProperty("Hatching/separationintervals", separationintervals);
    }

    void readOptionSetting(const KisPropertiesConfigurationSP setting) {
        angle = setting->getDouble("Hatching/angle");
        separation = setting->getDouble("Hatching/separation");
        thickness = setting->getDouble("Hatching/thickness");
        origin_x = setting->getDouble("Hatching/origin_x");
        origin_y = setting->getDouble("Hatching/origin_y");

        bool_nocrosshatching = setting->getBool("Hatching/bool_nocrosshatching");
        bool_perpendicular = setting->getBool("Hatching/bool_perpendicular");
        bool_minusthenplus = setting->getBool("Hatching/bool_minusthenplus");
        bool_plusthenminus = setting->getBool("Hatching/bool_plusthenminus");
        bool_moirepattern = setting->getBool("Hatching/bool_moirepattern");

        separationintervals = setting->getInt("Hatching/separationintervals");
    }

};

#endif
