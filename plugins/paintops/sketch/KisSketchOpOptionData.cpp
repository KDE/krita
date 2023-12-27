/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSketchOpOptionData.h"

#include "kis_properties_configuration.h"
#include <kis_paintop_lod_limitations.h>


const QString SKETCH_PROBABILITY = "Sketch/probability";
const QString SKETCH_DISTANCE_DENSITY = "Sketch/distanceDensity";
const QString SKETCH_OFFSET = "Sketch/offset";
const QString SKETCH_USE_SIMPLE_MODE = "Sketch/simpleMode";
const QString SKETCH_MAKE_CONNECTION = "Sketch/makeConnection";
const QString SKETCH_MAGNETIFY = "Sketch/magnetify";
const QString SKETCH_LINE_WIDTH = "Sketch/lineWidth";
const QString SKETCH_RANDOM_RGB = "Sketch/randomRGB";
const QString SKETCH_RANDOM_OPACITY = "Sketch/randomOpacity";
const QString SKETCH_DISTANCE_OPACITY = "Sketch/distanceOpacity";
const QString SKETCH_ANTIALIASING = "Sketch/antiAliasing";


bool KisSketchOpOptionData::read(const KisPropertiesConfiguration *setting)
{
    probability = setting->getDouble(SKETCH_PROBABILITY, 0.50);
    offset = setting->getDouble(SKETCH_OFFSET, 30.0);
    lineWidth = setting->getInt(SKETCH_LINE_WIDTH, 1);
    simpleMode = setting->getBool(SKETCH_USE_SIMPLE_MODE, false);
    makeConnection = setting->getBool(SKETCH_MAKE_CONNECTION, true);
    magnetify = setting->getBool(SKETCH_MAGNETIFY, true);
    randomRGB = setting->getBool(SKETCH_RANDOM_RGB, false);
    randomOpacity = setting->getBool(SKETCH_RANDOM_OPACITY, false);
    distanceDensity = setting->getBool(SKETCH_DISTANCE_DENSITY, true);
    distanceOpacity = setting->getBool(SKETCH_DISTANCE_OPACITY, false);
    antiAliasing = setting->getBool(SKETCH_ANTIALIASING, false);

    return true;
}

void KisSketchOpOptionData::write(KisPropertiesConfiguration *setting) const
{
    setting->setProperty(SKETCH_PROBABILITY, probability);
    setting->setProperty(SKETCH_OFFSET, offset);
    setting->setProperty(SKETCH_LINE_WIDTH, lineWidth);
    setting->setProperty(SKETCH_USE_SIMPLE_MODE, simpleMode);
    setting->setProperty(SKETCH_MAKE_CONNECTION, makeConnection);
    setting->setProperty(SKETCH_MAGNETIFY, magnetify);
    setting->setProperty(SKETCH_RANDOM_RGB, randomRGB);
    setting->setProperty(SKETCH_RANDOM_OPACITY, randomOpacity);
    setting->setProperty(SKETCH_DISTANCE_DENSITY, distanceDensity);
    setting->setProperty(SKETCH_DISTANCE_OPACITY, distanceOpacity);
    setting->setProperty(SKETCH_ANTIALIASING, antiAliasing);
}

KisPaintopLodLimitations KisSketchOpOptionData::lodLimitations() const
{
    KisPaintopLodLimitations l;
    l.limitations << KoID("sketch-brush", i18nc("PaintOp instant preview limitation", "Sketch brush (differences in connecting lines are possible)"));
    return l;
}
