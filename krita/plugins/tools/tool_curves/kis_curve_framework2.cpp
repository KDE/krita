#include <QPointF>
#include <QList>

struct KisCurvePoint {
    QPointF point;
    int pivot;
};

typedef QList<KisCurvePoint> KisPointList;
typedef QMutableListIterator<KisCurvePoint> KisBaseIterator;

class KisCurveIterator : public KisBaseIterator {

    typedef KisBaseIterator super;

public:

    KisCurveIterator (KisPointList& base) : super (base) {return;}
    ~KisCurveIterator () {return;}

    KisCurvePoint& nextPivot();
    KisCurvePoint& previousPivot();

};

inline KisCurvePoint& KisCurveIterator::nextPivot()
{
    while (hasNext())
        if (next().pivot)
            break;

    return value();
}

inline KisCurvePoint& KisCurveIterator::previousPivot()
{
    while (hasPrevious())
        if (previous().pivot)
            break;

    return value();
}
