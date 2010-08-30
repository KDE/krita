/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_SKETCHOP_OPTION_H
#define KIS_SKETCHOP_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

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
    ~KisSketchOpOption();

    void setThreshold(int radius) const;
    int threshold() const;

    void writeOptionSetting(KisPropertiesConfiguration* settings) const;
    void readOptionSetting(const KisPropertiesConfiguration* settings);


private:

    KisSketchOpOptionsWidget * m_options;

};

class SketchProperties{
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
    
    void readOptionSetting(const KisPropertiesConfiguration* settings){
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
