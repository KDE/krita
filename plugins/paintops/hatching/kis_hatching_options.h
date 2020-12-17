/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
