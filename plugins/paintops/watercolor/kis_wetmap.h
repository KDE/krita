#ifndef KIS_WETMAP_H
#define KIS_WETMAP_H

#include "kis_paint_device.h"

class KisWetMap
{
public:
    KisWetMap();

    void addWater(QPoint pos, qreal radius);
    void update();
    int getWater(int x, int y);
private:
    KisPaintDeviceSP m_wetMap;
    float* m_waterVelocitiesX;
    float* m_waterVelocitiesY;
    int m_width;
    int m_height;
};

#endif
