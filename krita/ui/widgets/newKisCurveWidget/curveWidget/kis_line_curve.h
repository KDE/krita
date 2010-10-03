#ifndef KIS_LINE_CURVE_H
#define KIS_LINE_CURVE_H

#include "kis_simple_curve.h"

class KisLineCurve : public KisSimpleCurve
{
public:
    KisLineCurve();

    QString className() const;
    void updatePainterPath();
};

#endif // KIS_LINE_CURVE_H
