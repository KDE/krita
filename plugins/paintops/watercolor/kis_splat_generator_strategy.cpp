#include "kis_splat_generator_strategy.h"


void KisSimpleBrushGenerator::generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, KoColor color)
{
    KisSplat *newSplat = new KisSplat(pos, radius, color);
    wetMap->addWater(pos, radius);
    flowing->insert(newSplat->boundingRect(), newSplat);
}

void KisWetOnDryGenerator::generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, KoColor color)
{
    wetMap->addWater(pos, radius);

    /// 1 splat in center, 6 around
    qreal radialSpeed = 2.f;
    int d = radius / 2;
    int r = d / 2;
    QVector2D offset;

    KisSplat *splat = new KisSplat(pos, radius, color);
    flowing->insert(splat->boundingRect(), splat);

    for (int i = 0; i < 6; i++) {
        qreal theta = i * M_PI / 3;
        offset.setX(r * cos(theta));
        offset.setY(r * sin(theta));

        splat = new KisSplat(pos + offset,
                             0.1f * radialSpeed * offset.normalized().toPointF(),
                             d,
                             30,
                             0.5f * radialSpeed,
                             1.f,
                             radialSpeed,
                             color);
        flowing->insert(splat->boundingRect(), splat);
    }
}

void KisCrunchyGenerator::generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, KoColor color)
{
    wetMap->addWater(pod, radius);

    KisSplat *splat = new KisSplat(pos,
                                      QPointF(0, 0),
                                      radius,
                                      15, 5, 0.25f, 2.f,
                                      color);
    flowing->insert(splat->boundingRect(), splat);
}

void KisWetOnWetGenerator::generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, KoColor color)
{
    wetMap->addWater(pod, radius);

    int smallD = radius / 2;
    int bigD = 3 * radius / 2;

    KisSplat *splat = new KisSplat(pos,
                                   QPointF(0, 0),
                                   bigD,
                                   15, 5, 1.f, 2.f,
                                   color);
    flowing->insert(splat->boundingRect(), splat);

    splat = new KisSplat(pos,
                         QPointF(0, 0),
                         smallD,
                         15, 5, 1.f, 2.f,
                         color);
    flowing->insert(splat->boundingRect(), splat);
}

void KisBlobbyGenerator::generate(KoRTree<KisSplat *> *flowing, KisWetMap *wetMap, QPointF pos, qreal radius, KoColor color)
{
    wetMap->addWater(pos, radius);

    qreal firstD = (qreal) radius / 3;
    qreal lastD = (qreal) radius;
    KisSplat *splat;

    for (int i = 0; i < 4; i++) {
        qreal size = get_random(firstD, lastD);
        QPointF posN;
        switch (i) {
        case 0:
            posN = pos + QPointF(0, (radius - size) / 2);
            break;
        case 1:
            posN = pos + QPointF((radius - size) / 2, 0);
        case 2:
            posN = pos - QPointF(0, (radius - size) / 2);
            break;
        case 3:
            posN = pos - QPointF((radius - size) / 2, 0);
        default:
            break;
        }

        splat = new KisSplat(posN,
                             QPointF(0, 0),
                             size,
                             15, 5, 1.f, 2.f,
                             color);
        flowing->insert(splat->boundingRect(), splat);
    }
}
