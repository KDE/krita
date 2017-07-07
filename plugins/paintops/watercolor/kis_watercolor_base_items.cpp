#include "kis_watercolor_base_items.h"

#include "kis_splat_generator_strategy.h"

void KisWatercolorBaseItems::paint(QPointF pos, qreal radius, KoColor color, int brushType)
{
    KisSplatGeneratorStrategy *strategy;
    switch (brushType) {
    case 0:
        strategy = new KisSimpleBrushGenerator();
        break;
    case 1:
        strategy = new KisWetOnDryGenerator();
        break;
    case 2:
        strategy = new KisWetOnWetGenerator();
        break;
    case 3:
        strategy = new KisBlobbyGenerator();
        break;
    case 4:
        strategy = new KisCrunchyGenerator();
        break;
    default:
        break;
    }

    strategy->generate(m_flowing, m_wetMap, pos, radius, color);
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
    m_updater.start(33);
    connect(&m_updater, SIGNAL(timeout()), SLOT(update()));
}
