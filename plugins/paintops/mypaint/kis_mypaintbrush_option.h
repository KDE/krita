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

const KoID SensorsListId("sensorslist", "SHOULD NOT APPEAR IN THE UI !"); ///< this a non user-visible sensor that can store a list of other sensors, and multiply their output

class KisMyPaintBrushOption;
typedef KisSharedPtr<KisMyPaintBrushOption> KisDynamicOptionSP;

enum MyPaintBrushOptionType {

    PRESSURE,
    FINE_SPEED,
    GROSS_SPEED,
    RANDOM,
    STROKE,
    DIRECTION,
    DECLINATION,
    ASCENSION,
    UNKNOWN = 255
};

class PAINTOP_EXPORT KisMyPaintBrushOption : public KisSerializableConfiguration
{

public:
    enum ParameterSign {
        NegativeParameter = -1,
        UnSignedParameter = 0,
        PositiveParameter = 1
    };

public:

    KisMyPaintBrushOption(MyPaintBrushOptionType type);
    ~KisMyPaintBrushOption() override;

    qreal parameter(const KisPaintInformation& info);

    qreal parameter(const KisPaintInformation& info, const KisCubicCurve curve, const bool customCurve);

    void reset();

    QWidget* createConfigurationWidget(QWidget* parent, QWidget* selector);

    static KisDynamicOptionSP id2Sensor(const KoID& id, const QString &parentOptionName);
    static KisDynamicOptionSP id2Sensor(const QString& s, const QString &parentOptionName) {
        return id2Sensor(KoID(s), parentOptionName);
    }

    static MyPaintBrushOptionType id2Type(const KoID& id);
    static MyPaintBrushOptionType id2Type(const QString& s) {
        return id2Type(KoID(s));
    }

    static KisDynamicOptionSP type2Sensor(MyPaintBrushOptionType sensorType, const QString &parentOptionName);

    static QString minimumLabel(MyPaintBrushOptionType sensorType);
    static QString maximumLabel(MyPaintBrushOptionType sensorType, int max = -1);
    static int minimumValue(MyPaintBrushOptionType sensorType);
    static int maximumValue(MyPaintBrushOptionType sensorType, int max = -1);
    static QString valueSuffix(MyPaintBrushOptionType sensorType);

    static KisDynamicOptionSP createFromXML(const QString&, const QString &parentOptionName);
    static KisDynamicOptionSP createFromXML(const QDomElement&, const QString &parentOptionName);

    static QList<KoID> sensorsIds();
    static QList<MyPaintBrushOptionType> sensorsTypes();
    static MyPaintBrushOptionType typeForInput(MyPaintBrushInput input);

    static QString id(MyPaintBrushOptionType sensorType);

    using KisSerializableConfiguration::fromXML;
    using KisSerializableConfiguration::toXML;

    void toXML(QDomDocument&, QDomElement&) const override;
    void fromXML(const QDomElement&) override;

    void setCurve(const KisCubicCurve& curve);
    const KisCubicCurve& curve() const;
    void removeCurve();
    bool hasCustomCurve() const;

    void setActive(bool active);
    bool isActive() const;

    bool dependsOnCanvasRotation() const;

    bool isAdditive() const;
    bool isAbsoluteRotation() const;

    inline MyPaintBrushOptionType sensorType() const { return m_type; }

    MyPaintBrushInput input();

    int length() { return m_length; }


public:
    static inline qreal scalingToAdditive(qreal x) {
        return -1.0 + 2.0 * x;
    }

    static inline qreal additiveToScaling(qreal x) {
        return 0.5 * (1.0 + x);
    }

protected:

    qreal value(const KisPaintInformation& info);

    int m_length;

private:

    Q_DISABLE_COPY(KisMyPaintBrushOption)

    MyPaintBrushOptionType m_type;
    bool m_customCurve;
    KisCubicCurve m_curve;
    bool m_active;

};

#endif // KIS_MYPAINTBRUSH_OPTION_H
