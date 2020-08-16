#ifndef KIS_MYPAINTBRUSH_OPTION_H
#define KIS_MYPAINTBRUSH_OPTION_H

#include <kritapaintop_export.h>

#include <QObject>

#include <KoID.h>

#include <klocalizedstring.h>

#include "kis_serializable_configuration.h"
#include "kis_curve_label.h"
#include <kis_cubic_curve.h>
#include <kis_shared_ptr.h>
#include <kis_shared.h>
#include <kis_dynamic_sensor.h>
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

class KisMyPaintBrushOption;

class PAINTOP_EXPORT KisMyPaintBrushOption : public KisDynamicSensor
{

public:

    KisMyPaintBrushOption(DynamicSensorType type);
    ~KisMyPaintBrushOption() override;

    qreal value(const KisPaintInformation &info) override;
    static KisDynamicSensorSP id2Sensor(const KoID& id, const QString &parentOptionName);
    static KisDynamicSensorSP id2Sensor(const QString& s, const QString &parentOptionName) {
        return id2Sensor(KoID(s), parentOptionName);
    }

    static DynamicSensorType id2Type(const KoID& id);
    static DynamicSensorType id2Type(const QString& s) {
        return id2Type(KoID(s));
    }

    static KisDynamicSensorSP type2Sensor(DynamicSensorType sensorType, const QString &parentOptionName);

    static QString minimumLabel(DynamicSensorType sensorType);
    static QString maximumLabel(DynamicSensorType sensorType, int max = -1);
    static int minimumValue(DynamicSensorType sensorType);
    static int maximumValue(DynamicSensorType sensorType, int max = -1);
    static QString valueSuffix(DynamicSensorType sensorType);

    static QList<KoID> sensorsIds();
    static QList<DynamicSensorType> sensorsTypes();
    static DynamicSensorType typeForInput(MyPaintBrushInput input);

    QString id(DynamicSensorType sensorType);
    QString id();

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
