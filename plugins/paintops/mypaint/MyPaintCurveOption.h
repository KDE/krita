/*
 *  SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KIS_MYPAINT_CURVE_OPTION_H
#define KIS_MYPAINT_CURVE_OPTION_H


#include <QObject>
#include <QVector>

#include "kis_paintop_option.h"
#include "kis_global.h"
#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include "kritapaintop_export.h"
#include "MyPaintBrushOption.h"
#include "libmypaint/mypaint-brush.h"

class KisMyPaintCurveOption: public KisCurveOption
{
    Q_OBJECT
public:
    KisMyPaintCurveOption(const QString& name,
                   KisPaintOpOption::PaintopCategory category,
                   bool checked,
                   qreal value = 1.0,
                   qreal min = 0.0,
                   qreal max = 1.0);

    virtual ~KisMyPaintCurveOption();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(KisPropertiesConfigurationSP setting) override;
    void lodLimitations(KisPaintopLodLimitations *l) const override;

    MyPaintBrushSetting currentSetting();
    QList<MyPaintBrushInput> inputList();

    QList<KoID> sensorsIds() override;
    DynamicSensorType id2Type(const KoID &id) override;
    KisDynamicSensorSP id2Sensor(const KoID &id, const QString &parentOptionName) override;
    QList<DynamicSensorType> sensorsTypes() override;
    KisDynamicSensorSP type2Sensor(DynamicSensorType sensorType, const QString &parentOptionName) override;

protected:
    void readNamedOptionSetting(const QString &prefix, const KisPropertiesConfigurationSP setting) override;

private:
    bool firstRead = true;

};

#endif // KIS_MYPAINT_CURVE_OPTION_H
