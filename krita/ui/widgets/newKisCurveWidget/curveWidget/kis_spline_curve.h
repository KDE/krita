#ifndef KIS_SPLINE_CURVE_H
#define KIS_SPLINE_CURVE_H

#include "kis_simple_curve.h"

class KisSplineCurve : public KisSimpleCurve
{
public:
    KisSplineCurve();

    QString className() const;
    void updatePainterPath();
};

#endif // KIS_SPLINE_CURVE_H
