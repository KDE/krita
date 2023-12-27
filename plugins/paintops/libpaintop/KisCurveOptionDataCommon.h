/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCURVEOPTIONDATACOMMON_H
#define KISCURVEOPTIONDATACOMMON_H

#include <boost/operators.hpp>

#include "kis_assert.h"
#include "kis_paintop_option.h"
#include "KisDynamicSensorIds.h"
#include "kritapaintop_export.h"

#include <KisSensorPackInterface.h>


struct PAINTOP_EXPORT KisCurveOptionDataCommon : boost::equality_comparable<KisCurveOptionDataCommon>
{
    static constexpr bool supports_prefix = true;

    KisCurveOptionDataCommon(const QString &prefix,
                            const KoID &id,
                            bool isCheckable,
                            bool isChecked,
                            qreal minValue,
                            qreal maxValue,
                            KisSensorPackInterface *sensorInterface);

    KisCurveOptionDataCommon(const KoID &id,
                            bool isCheckable,
                            bool isChecked,
                            qreal minValue,
                            qreal maxValue,
                            KisSensorPackInterface *sensorInterface);

    inline friend bool operator==(const KisCurveOptionDataCommon &lhs, const KisCurveOptionDataCommon &rhs) {
        return lhs.id == rhs.id &&
                lhs.prefix == rhs.prefix &&
                lhs.isCheckable == rhs.isCheckable &&
                lhs.isChecked == rhs.isChecked &&
                lhs.useCurve == rhs.useCurve &&
                lhs.useSameCurve == rhs.useSameCurve &&
                lhs.curveMode == rhs.curveMode &&
                lhs.commonCurve == rhs.commonCurve &&
                lhs.strengthValue == rhs.strengthValue &&
                lhs.strengthMinValue == rhs.strengthMinValue &&
                lhs.strengthMaxValue == rhs.strengthMaxValue &&
                lhs.sensorData->compare(rhs.sensorData.constData());
    }

    KoID id;
    QString prefix;
    bool isCheckable = true;
    qreal strengthMinValue = 0.0;
    qreal strengthMaxValue = 1.0;

    bool isChecked = true;
    bool useCurve = true;
    bool useSameCurve = true;

    int curveMode = 0; // TODO: to enum!
    QString commonCurve = DEFAULT_CURVE_STRING;
    qreal strengthValue = 1.0;

    QSharedDataPointer<KisSensorPackInterface> sensorData;

    std::vector<const KisSensorData *> sensors() const;
    std::vector<KisSensorData*> sensors();

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

    using ValueFixUpReadCallback = std::function<void (KisCurveOptionDataCommon *, const KisPropertiesConfiguration *)>;
    ValueFixUpReadCallback valueFixUpReadCallback;

    using ValueFixUpWriteCallback= std::function<void (qreal, KisPropertiesConfiguration *)>;
    ValueFixUpWriteCallback valueFixUpWriteCallback;

private:
    bool readPrefixed(const KisPropertiesConfiguration *setting);
    void writePrefixed(KisPropertiesConfiguration *setting) const;
};

#endif // KISCURVEOPTIONDATACOMMON_H
