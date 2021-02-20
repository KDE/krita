/*
 * SPDX-FileCopyrightText: 2008 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2011 Silvio Heinrich <plassy@web.de>
 * SPDX-FileCopyrightText: 2020 Ashwin Dhakaita <ashwingpdhakaita@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_MYPAINTBRUSH_OPTION_H
#define KIS_MYPAINTBRUSH_OPTION_H

#include <kritapaintop_export.h>

#include <QObject>

#include <KoID.h>
#include <kis_cubic_curve.h>
#include <kis_curve_label.h>
#include <kis_dynamic_sensor.h>
#include <kis_serializable_configuration.h>
#include <kis_shared.h>
#include <kis_shared_ptr.h>
#include <klocalizedstring.h>
#include <kritapaintop_export.h>
#include <libmypaint/mypaint-brush.h>

class QWidget;
class KisPaintInformation;

const KoID Pressure("pressure", ki18n("Pressure"));
const KoID FineSpeed("speed1", ki18n("Fine Speed"));
const KoID GrossSpeed("speed2", ki18n("Gross Speed"));
const KoID Random("random", ki18n("Random"));
const KoID Stroke("stroke", ki18n("Stroke"));
const KoID Direction("direction", ki18n("Direction"));
const KoID Declination("tilt_declination", ki18n("Declination"));
const KoID Ascension("tilt_ascension", ki18n("Ascension"));
const KoID Custom("custom", ki18n("Custom"));

class KisMyPaintBrushOption : public KisDynamicSensor
{

public:

    KisMyPaintBrushOption(DynamicSensorType type);
    ~KisMyPaintBrushOption() override;

    qreal value(const KisPaintInformation &info) override;

    QString minimumLabel(DynamicSensorType sensorType) override;
    QString maximumLabel(DynamicSensorType sensorType, int max = -1) override;
    int minimumValue(DynamicSensorType sensorType) override;
    int maximumValue(DynamicSensorType sensorType, int max = -1) override;
    QString valueSuffix(DynamicSensorType sensorType) override;

    static DynamicSensorType typeForInput(MyPaintBrushInput input);

    QString id(DynamicSensorType sensorType);

    void setCurveFromPoints(QList<QPointF> points);

    inline DynamicSensorType sensorType() const { return m_type; }

    MyPaintBrushInput input();

    int length() { return m_length; }

    qreal getXRangeMin();
    qreal getXRangeMax();
    qreal getYRangeMin();
    qreal getYRangeMax();

    void setXRangeMin(qreal value);
    void setXRangeMax(qreal value);
    void setYRangeMin(qreal value);
    void setYRangeMax(qreal value);

public:
    static inline qreal scalingToAdditive(qreal x) {
        return -1.0 + 2.0 * x;
    }

    static inline qreal additiveToScaling(qreal x) {
        return 0.5 * (1.0 + x);
    }

    QList<QPointF> getControlPoints();
    QString minimumXLabel();
    QString minimumYLabel();
    QString maximumXLabel();
    QString maximumYLabel();

protected:

    QPointF scaleTo0_1(QPointF point);
    QPointF scaleFrom0_1(QPointF point);
    qreal scaleToRange(qreal inMin, qreal inMax, qreal outMin, qreal outMax, qreal inValue);
    void setRangeFromPoints(QList<QPointF> points);

    qreal curveXMin = 0;
    qreal curveXMax = 1;
    qreal curveYMin = 0;
    qreal curveYMax = 1;

private:
    Q_DISABLE_COPY(KisMyPaintBrushOption)

};

#endif // KIS_MYPAINTBRUSH_OPTION_H
