/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHatchingOptionsData.h"

#include "kis_properties_configuration.h"
#include <kis_paintop_lod_limitations.h>


const QString HATCHING_ANGLE = "Hatching/angle";
const QString HATCHING_SEPARATION = "Hatching/separation";
const QString HATCHING_THICKNESS = "Hatching/thickness";
const QString HATCHING_ORIGIN_X = "Hatching/origin_x";
const QString HATCHING_ORIGIN_Y = "Hatching/origin_y";

const QString HATCHING_BOOL_NOCROSSHATCHING = "Hatching/bool_nocrosshatching";
const QString HATCHING_BOOL_PERPENDICULAR = "Hatching/bool_perpendicular";
const QString HATCHING_BOOL_MINUSTHENPLUS = "Hatching/bool_minusthenplus";
const QString HATCHING_BOOL_PLUSTHENMINUS = "Hatching/bool_plusthenminus";
const QString HATCHING_BOOL_MOIREPATTERN = "Hatching/bool_moirepattern";

const QString HATCHING_SEPARATIONINTERVALS = "Hatching/separationintervals";


bool KisHatchingOptionsData::read(const KisPropertiesConfiguration *setting)
{
        angle = setting->getDouble(HATCHING_ANGLE, -60.0);
        separation = setting->getDouble(HATCHING_SEPARATION, 6.0);
        thickness = setting->getDouble(HATCHING_THICKNESS, 1.0);
        originX = setting->getDouble(HATCHING_ORIGIN_X, 50.0);
        originY = setting->getDouble(HATCHING_ORIGIN_Y, 50.0);

        if (setting->getBool(HATCHING_BOOL_NOCROSSHATCHING, true)) {
            crosshatchingStyle = CrosshatchingType::NoCrosshatching;
        }
        else if (setting->getBool(HATCHING_BOOL_PERPENDICULAR, false)) {
            crosshatchingStyle = CrosshatchingType::Perpendicular;
        }
        else if (setting->getBool(HATCHING_BOOL_MINUSTHENPLUS, false)) {
            crosshatchingStyle = CrosshatchingType::MinusThenPlus;
        }
        else if (setting->getBool(HATCHING_BOOL_PLUSTHENMINUS, false)) {
            crosshatchingStyle = CrosshatchingType::PlusThenMinus;
        }
        else if (setting->getBool(HATCHING_BOOL_MOIREPATTERN, false)) {
            crosshatchingStyle = CrosshatchingType::MoirePattern;
        }

        separationIntervals = setting->getInt(HATCHING_SEPARATIONINTERVALS, 2);
    return true;
}

void KisHatchingOptionsData::write(KisPropertiesConfiguration *setting) const
{
        setting->setProperty(HATCHING_ANGLE, angle);
        setting->setProperty(HATCHING_SEPARATION, separation);
        setting->setProperty(HATCHING_THICKNESS, thickness);
        setting->setProperty(HATCHING_ORIGIN_X, originX);
        setting->setProperty(HATCHING_ORIGIN_Y, originY);

        setting->setProperty(HATCHING_BOOL_NOCROSSHATCHING, crosshatchingStyle == CrosshatchingType::NoCrosshatching);
        setting->setProperty(HATCHING_BOOL_PERPENDICULAR, crosshatchingStyle == CrosshatchingType::Perpendicular);
        setting->setProperty(HATCHING_BOOL_MINUSTHENPLUS, crosshatchingStyle == CrosshatchingType::MinusThenPlus);
        setting->setProperty(HATCHING_BOOL_PLUSTHENMINUS, crosshatchingStyle == CrosshatchingType::PlusThenMinus);
        setting->setProperty(HATCHING_BOOL_MOIREPATTERN, crosshatchingStyle == CrosshatchingType::MoirePattern);

        setting->setProperty(HATCHING_SEPARATIONINTERVALS, separationIntervals);
}

KisPaintopLodLimitations KisHatchingOptionsData::lodLimitations() const
{
    KisPaintopLodLimitations l;
    l.limitations << KoID("hatching-brush", i18nc("PaintOp instant preview limitation", "Hatching Brush (heavy aliasing in preview mode)"));
    return l;
}
