#include "kis_watercolor_base_items.h"

void KisWatercolorBaseItems::paint(QPointF pos, qreal radius, KoColor &color)
{
    m_wetMap->addWater(pos.toPoint(), radius);
    KisSplat *splat = new KisSplat(pos, radius / 2, color);
    m_flowing.insert(splat->boundingRect(), splat);
}

void KisWatercolorBaseItems::update()
{
    QList<KisSplat *> list = m_flowing.values();

    foreach (KisSplat *splat, list) {
        if (splat->update(m_wetMap) == KisSplat::Fixed) {
            m_fixed.insert(splat->boundingRect(), splat);
            m_flowing.remove(splat);
        }
    }
    list = m_fixed.values();
    foreach (KisSplat *splat, list) {
        if (splat->update(m_wetMap) == KisSplat::Dried) {
            m_dried.insert(splat->boundingRect(), splat);
            m_fixed.remove(splat);
        }
    }

    m_wetMap->update();
}

KisWatercolorBaseItems::KisWatercolorBaseItems() : m_flowing(4, 2), m_fixed(4, 2), m_dried(4, 2)
{
    m_wetMap = new KisWetMap();
}
