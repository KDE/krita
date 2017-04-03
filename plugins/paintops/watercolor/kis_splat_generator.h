#ifndef KIS_SPLAT_GENERATOR
#define KIS_SPLAT_GENERATOR

#include <QVector>
#include "kis_wetmap.h"
#include "kis_splat.h"

#include "KoColor.h"
#include "kis_types.h"

class SplatGenerator
{
public:
    SplatGenerator(int width, KoColor &clr, KisPaintDeviceSP dev);

    void generateFromPoints(QVector<QPointF> &points, int msec);

private:
    QVector<KisSplat*> *m_flowing;
    QVector<KisSplat*> *m_fixed;
    QVector<KisSplat*> *m_dried;

    KisWetMap *m_wetMap;

    int m_width;
    KoColor m_color;
    KisPaintDeviceSP m_device;
};

#endif
