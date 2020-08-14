#ifndef KIS_MYPAINT_CURVE_OPTION_H
#define KIS_MYPAINT_CURVE_OPTION_H


#include <QObject>
#include <QVector>

#include "kis_paintop_option.h"
#include "kis_global.h"
#include "kis_curve_option.h"
#include <brushengine/kis_paint_information.h>
#include "kritapaintop_export.h"
#include "kis_mypaintbrush_option.h"
#include "libmypaint/mypaint-brush.h"

class PAINTOP_EXPORT KisMyPaintCurveOption: public QObject, public KisCurveOption
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

    virtual void writeOptionSetting(KisPropertiesConfigurationSP setting);
    virtual void readOptionSetting(KisPropertiesConfigurationSP setting);    

    void resetAllSensors();
    KisDynamicSensorSP sensor(DynamicSensorType sensorType, bool active) const;
    void replaceSensor(KisDynamicSensorSP sensor);
    QList<KisDynamicSensorSP> sensors();
    QList<KisDynamicSensorSP> activeSensors() const;
    MyPaintBrushSetting currentSetting();
    QList<MyPaintBrushInput> inputList();

    void setCurve(DynamicSensorType sensorType, bool useSameCurve, const KisCubicCurve &curve);

    KisMyPaintCurveOption::ValueComponents computeValueComponents(const KisPaintInformation& info) const;

protected:

    void readNamedOptionSetting(const QString& prefix, const KisPropertiesConfigurationSP setting);
    QMap<DynamicSensorType, KisDynamicSensorSP> m_sensorMap;

private:

    qreal m_value;
    qreal m_minValue;
    qreal m_maxValue;
    bool firstRead = true;

Q_SIGNALS:
    void unCheckUseCurve();
};

#endif // KIS_MYPAINT_CURVE_OPTION_H
