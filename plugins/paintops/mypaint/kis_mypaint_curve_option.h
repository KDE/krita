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

class PAINTOP_EXPORT KisMyPaintCurveOption: public KisCurveOption
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

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const;
    void readOptionSetting(KisPropertiesConfigurationSP setting);
    void lodLimitations(KisPaintopLodLimitations *l) const override;

    MyPaintBrushSetting currentSetting();
    QList<MyPaintBrushInput> inputList();

    QList<KoID> sensorsIds();
    DynamicSensorType id2Type(const KoID &id);
    KisDynamicSensorSP id2Sensor(const KoID& id, const QString &parentOptionName);
    QList<DynamicSensorType> sensorsTypes();
    KisDynamicSensorSP type2Sensor(DynamicSensorType sensorType, const QString &parentOptionName);

protected:

    void readNamedOptionSetting(const QString& prefix, const KisPropertiesConfigurationSP setting);   

    qreal m_value;
    qreal m_minValue;
    qreal m_maxValue;

private:
    bool firstRead = true;

};

#endif // KIS_MYPAINT_CURVE_OPTION_H
