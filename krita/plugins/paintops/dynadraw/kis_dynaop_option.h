/*
 *  Copyright (c) 2009-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_DYNAOP_OPTION_H
#define KIS_DYNAOP_OPTION_H

#include <kis_paintop_option.h>

const QString DYNA_WIDTH = "Dyna/width";
const QString DYNA_MASS = "Dyna/mass";
const QString DYNA_DRAG = "Dyna/drag";
const QString DYNA_USE_FIXED_ANGLE = "Dyna/useFixedAngle";
const QString DYNA_X_ANGLE = "Dyna/xAngle";
const QString DYNA_Y_ANGLE = "Dyna/yAngle";
const QString DYNA_WIDTH_RANGE = "Dyna/widthRange";
const QString DYNA_ACTION = "Dyna/action";
const QString DYNA_CIRCLE_RADIUS = "Dyna/circleRadius";
const QString DYNA_USE_TWO_CIRCLES = "Dyna/useTwoCirles";
const QString DYNA_ENABLE_LINE = "Dyna/enableLine";
const QString DYNA_LINE_COUNT = "Dyna/lineCount";
const QString DYNA_LINE_SPACING = "Dyna/lineSpacing";

class KisDynaOpOptionsWidget;

class KisDynaOpOption : public KisPaintOpOption
{
public:
    KisDynaOpOption();
    ~KisDynaOpOption();

    qreal initWidth() const;
    qreal mass() const;
    qreal drag() const;
    bool useFixedAngle() const;
    qreal xAngle() const;
    qreal yAngle() const;
    qreal widthRange() const;

    int action() const;
    int circleRadius() const;
    bool enableLine() const;
    bool useTwoCircles() const;

    int lineCount() const;
    qreal lineSpacing() const;

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
    KisDynaOpOptionsWidget * m_options;

};

#endif
