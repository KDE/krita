class KisCubicCurvePoint
{
%TypeHeaderCode
#include "kis_cubic_curve.h"
%End
public:
    KisCubicCurvePoint();
    KisCubicCurvePoint(const KisCubicCurvePoint&);
    KisCubicCurvePoint(qreal x, qreal y, bool setAsCorner = false);
    bool operator==(const KisCubicCurvePoint &other) const;
    qreal x() const;
    qreal y() const;
    const QPointF& position() const;
    bool isSetAsCorner() const;
    void setX(qreal newX);
    void setY(qreal newY);
    void setPosition(const QPointF &newPosition);
    void setAsCorner(bool newIsSetAsCorner);
};

class KisCubicCurve
{
%TypeHeaderCode
#include "kis_cubic_curve.h"
%End
public:
    KisCubicCurve();
    KisCubicCurve(const QList<QPointF>& points);
    KisCubicCurve(const QList<KisCubicCurvePoint>& points);
    KisCubicCurve(const QString &curveString);
    KisCubicCurve(const KisCubicCurve& curve);
    qreal value(qreal x) const;
    QList<QPointF> points() const;
    QList<KisCubicCurvePoint> curvePoints() const;
    void setPoints(const QList<QPointF>& points);
    void setPoints(const QList<KisCubicCurvePoint>& points);
    void setPoint(int idx, const KisCubicCurvePoint& point);
    void setPoint(int idx, const QPointF& position, bool setAsCorner);
    void setPoint(int idx, const QPointF& point);
    void setPointPosition(int idx, const QPointF& position);
    void setPointAsCorner(int idx, bool setAsCorner);
    int addPoint(const KisCubicCurvePoint& point);
    int addPoint(const QPointF& position, bool setAsCorner);
    int addPoint(const QPointF& point);
    void removePoint(int idx);
    bool isIdentity() const;
    bool isConstant(qreal x) const;
    void setName(const QString& name);
    const QString& name() const;
    static qreal interpolateLinear(qreal normalizedValue, const QVector<qreal> &transfer);
    const QVector<quint16> uint16Transfer(int size = 256) const;
    const QVector<qreal> floatTransfer(int size = 256) const;
    QString toString() const;
    void fromString(const QString&);
};
