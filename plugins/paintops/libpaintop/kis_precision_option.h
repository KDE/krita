/*
 *  SPDX-FileCopyrightText: 2012 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2014 Mohit Goyal <mohit.bits2011@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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


namespace KisBrushModel {
struct PrecisionData :  public boost::equality_comparable<PrecisionData>
{
    int precisionLevel = 5;
    bool useAutoPrecision = false;

    friend bool operator==(const PrecisionData &lhs, const PrecisionData &rhs);
    static PrecisionData read(const KisPropertiesConfiguration *config);
    void write(KisPropertiesConfiguration *config) const;
};
}

class PAINTOP_EXPORT KisPrecisionOption
{
public:
    KisPrecisionOption(const KisPropertiesConfiguration *setting);

    int effectivePrecisionLevel(qreal effectiveDabSize) const;
    void setHasImprecisePositionOptions(bool value);
    bool hasImprecisePositionOptions() const;

    int precisionLevel() const;
    void setPrecisionLevel(int precisionLevel);
    void setAutoPrecisionEnabled(int);
    bool autoPrecisionEnabled();

private:
    bool m_hasImprecisePositionOptions {false};
    KisBrushModel::PrecisionData m_precisionData;
};

#endif /* __KIS_PRECISION_OPTION_H */
