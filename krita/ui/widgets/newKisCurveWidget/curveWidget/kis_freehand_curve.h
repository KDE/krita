#ifndef KIS_FREEHAND_CURVE_H
#define KIS_FREEHAND_CURVE_H

#include "kis_simple_curve.h"

class KisFreehandCurve : public KisSimpleCurve
{
public:
    KisFreehandCurve();

    QString className() const;
    void updatePainterPath();
};

#endif // KIS_FREEHAND_CURVE_H
