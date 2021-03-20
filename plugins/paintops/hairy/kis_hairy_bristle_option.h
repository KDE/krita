/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_HAIRY_BRISTLE_OPTION_H
#define KIS_HAIRY_BRISTLE_OPTION_H

#include <kis_paintop_option.h>
class KisPaintopLodLimitations;

const QString HAIRY_BRISTLE_USE_MOUSEPRESSURE = "HairyBristle/useMousePressure";
const QString HAIRY_BRISTLE_SCALE = "HairyBristle/scale";
const QString HAIRY_BRISTLE_SHEAR = "HairyBristle/shear";
const QString HAIRY_BRISTLE_RANDOM = "HairyBristle/random";
const QString HAIRY_BRISTLE_DENSITY = "HairyBristle/density";
const QString HAIRY_BRISTLE_THRESHOLD = "HairyBristle/threshold";
const QString HAIRY_BRISTLE_ANTI_ALIASING = "HairyBristle/antialias";
const QString HAIRY_BRISTLE_USE_COMPOSITING = "HairyBristle/useCompositing";
const QString HAIRY_BRISTLE_CONNECTED = "HairyBristle/isConnected";

class KisBristleOptionsWidget;

class KisHairyBristleOption : public KisPaintOpOption
{
public:
    KisHairyBristleOption();
    ~KisHairyBristleOption() override;

    void setScaleFactor(qreal scale) const;

    bool useMousePressure() const;

    double scaleFactor() const;
    double shearFactor() const;
    double randomFactor() const;

    void writeOptionSetting(KisPropertiesConfigurationSP config) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP config) override;
    void lodLimitations(KisPaintopLodLimitations *l) const override;

private:
    KisBristleOptionsWidget * m_options;
};

#endif // KIS_HAIRY_BRISTLE_OPTION_H

