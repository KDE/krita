/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_PRESSURE_MIRROR_OPTION_H
#define KIS_PRESSURE_MIRROR_OPTION_H

#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include <kritapaintop_export.h>
#include <kis_types.h>


struct MirrorProperties {
    MirrorProperties()
        : horizontalMirror(false),
          verticalMirror(false),
          coordinateSystemFlipped(false) {}

    bool horizontalMirror;
    bool verticalMirror;

    bool coordinateSystemFlipped;

    bool isEmpty() const {
        return !horizontalMirror && !verticalMirror;
    }
};

const QString MIRROR_HORIZONTAL_ENABLED = "HorizontalMirrorEnabled";
const QString MIRROR_VERTICAL_ENABLED = "VerticalMirrorEnabled";

/**
 * If the sensor value is higher then 0.5, then the related mirror option is true, false otherwise
 */
class PAINTOP_EXPORT KisPressureMirrorOption : public KisCurveOption
{
public:
    KisPressureMirrorOption();

    /**
    * Set the
    */
    MirrorProperties apply(const KisPaintInformation& info) const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

    void enableVerticalMirror(bool mirror);
    void enableHorizontalMirror(bool mirror);
    bool isVerticalMirrorEnabled();
    bool isHorizontalMirrorEnabled();

private:
    bool m_enableVerticalMirror;
    bool m_enableHorizontalMirror;
};

#endif
