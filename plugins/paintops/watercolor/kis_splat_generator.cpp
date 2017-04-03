#include "kis_splat_generator.h"
#include "kis_painter.h"

SplatGenerator::SplatGenerator(int width, KoColor &clr,
                               KisPaintDeviceSP dev) : m_width(width),
                                                       m_color(clr),
                                                       m_device(dev)
{
    m_wetMap = new KisWetMap();

    m_flowing = new QVector<KisSplat*>();
    m_fixed = new QVector<KisSplat*>();
    m_dried = new QVector<KisSplat*>();
}

void SplatGenerator::generateFromPoints(QVector<QPointF> &points, int msec)
{
    foreach (QPointF pnt, points) {
        m_flowing->push_back(new KisSplat(pnt, m_width, m_color));
        m_wetMap->addWater(pnt.toPoint(), m_width);
    }

    KisPainter painter(m_device);
    painter.setPaintColor(m_color);

    for (int i = 0; i <= msec; i +=33) {
        foreach (KisSplat *splat, *m_flowing) {
            painter.fillPainterPath(splat->shape());

            if (splat->update(m_wetMap) == KisSplat::Fixed) {
                m_fixed->push_back(splat);
                m_flowing->removeOne(splat);
            }
        }

        foreach (KisSplat *splat, *m_fixed) {
            if (splat->update(m_wetMap) == KisSplat::Dried) {
                m_dried->push_back(splat);
                m_fixed->removeOne(splat);
            }
        }

        m_wetMap->update();
    }
}
