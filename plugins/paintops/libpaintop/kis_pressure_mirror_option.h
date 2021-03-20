/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
