/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SKETCHOP_OPTION_H
#define KIS_SKETCHOP_OPTION_H

#include <kis_paintop_option.h>

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

class KisSketchOpOptionsWidget;

class KisSketchOpOption : public KisPaintOpOption
{
public:
    KisSketchOpOption();
    ~KisSketchOpOption() override;

    void setThreshold(int radius) const;
    int threshold() const;

    void writeOptionSetting(KisPropertiesConfigurationSP settings) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP settings) override;

    void lodLimitations(KisPaintopLodLimitations *l) const override;

private:

    KisSketchOpOptionsWidget * m_options;

};

class SketchProperties
{
public:
    qreal offset; // perc
    qreal probability; // perc
    bool simpleMode;
    bool makeConnection;
    bool magnetify;
    bool randomRGB;
    bool randomOpacity;
    bool distanceOpacity;
    bool distanceDensity;
    int lineWidth; // px

    void readOptionSetting(const KisPropertiesConfigurationSP settings) {
        probability = settings->getDouble(SKETCH_PROBABILITY);
        offset = settings->getDouble(SKETCH_OFFSET) * 0.01;
        lineWidth = settings->getInt(SKETCH_LINE_WIDTH);
        simpleMode = settings->getBool(SKETCH_USE_SIMPLE_MODE);
        makeConnection = settings->getBool(SKETCH_MAKE_CONNECTION);
        magnetify = settings->getBool(SKETCH_MAGNETIFY);
        randomRGB = settings->getBool(SKETCH_RANDOM_RGB);
        randomOpacity = settings->getBool(SKETCH_RANDOM_OPACITY);
        distanceDensity = settings->getBool(SKETCH_DISTANCE_DENSITY);
        distanceOpacity = settings->getBool(SKETCH_DISTANCE_OPACITY);
    }
};

#endif
