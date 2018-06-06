class KisCubicCurve
{
%TypeHeaderCode
#include "kis_cubic_curve.h"
%End
public:
    KisCubicCurve();
    qreal value(qreal x) const;
    QList<QPointF> points() const;
    void setPoints(const QList<QPointF>& points);
    void setPoint(int idx, const QPointF& point);
    int addPoint(const QPointF& point);
    void removePoint(int idx);
    bool isIdentity() const;
    bool isConstant(qreal x) const;
    void setName(const QString& name);
    const QString& name() const;
    QString toString() const;
    void fromString(const QString&);
};
