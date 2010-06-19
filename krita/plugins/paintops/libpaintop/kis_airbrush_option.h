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
#ifndef KIS_AIRBRUSH_OPTION_H
#define KIS_AIRBRUSH_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

const QString AIRBRUSH_ENABLED = "AirbrushOption/isAirbrushing";
const QString AIRBRUSH_RATE = "AirbrushOption/rate";

class KisAirbrushWidget;

/**
 * Allows the user to activate airbrushing of the brush mask (brush is painted at the same position over and over)
 * Rate is set in miliseconds.
 */
class PAINTOP_EXPORT KisAirbrushOption : public KisPaintOpOption
{
public:
    KisAirbrushOption(bool enabled = true);
    ~KisAirbrushOption();

    // return true if it's on
    bool airbrushing() const;
    // rate in miliseconds (delay between two brush masks are delivered )
    int rate() const;

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
    KisAirbrushWidget * m_optionWidget;

};

#endif
