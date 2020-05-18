/*
 *  Copyright (c) 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  Copyright (c) 2014 Mohit Goyal    <mohit.bits2011@gmail.com>
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

#ifndef __KIS_PRECISION_OPTION_H
#define __KIS_PRECISION_OPTION_H

#include <QString>
#include <kritapaintop_export.h>
#include <kis_properties_configuration.h>

const QString PRECISION_LEVEL = "KisPrecisionOption/precisionLevel";
const QString AUTO_PRECISION_ENABLED = "KisPrecisionOption/AutoPrecisionEnabled";
const QString STARTING_SIZE = "KisPrecisionOption/SizeToStartFrom";
const QString DELTA_VALUE = "KisPrecisionOption/DeltaValue";


class PAINTOP_EXPORT KisPrecisionOption
{
public:
    void writeOptionSetting(KisPropertiesConfigurationSP settings) const;
    void readOptionSetting(const KisPropertiesConfigurationSP settings);

    int effectivePrecisionLevel(qreal effectiveDabSize) const;
    void setHasImprecisePositionOptions(bool value);
    bool hasImprecisePositionOptions() const;

    int precisionLevel() const;
    void setPrecisionLevel(int precisionLevel);
    void setAutoPrecisionEnabled(int);
    bool autoPrecisionEnabled();

private:
    int m_precisionLevel;
    bool m_autoPrecisionEnabled;
    double m_sizeToStartFrom;
    double m_deltaValue;
    bool m_hasImprecisePositionOptions = false;
};

#endif /* __KIS_PRECISION_OPTION_H */
