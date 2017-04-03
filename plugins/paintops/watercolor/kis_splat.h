#ifndef KIS_SPLAT_H
#define KIS_SPLAT_H

#include <QPointF>
#include <QPolygonF>
#include <KoColor.h>
#include <QTime>
#include <QPainterPath>

#include "kis_wetmap.h"
#include "kis_random_generator.h"


class KisSplat
{
public:
    enum SplatState {
        Flowing,
        Fixed,
        Dried
    };

    KisSplat(QPointF offset, int width, KoColor splatColor);
    KisSplat(QPointF offset, QPointF velocityBias, int width, int life,
          qreal roughness, qreal flow, qreal radialSpeed, KoColor splatColor);

    qreal CalcSize();
    KoColor getColor();

    QPainterPath shape() const;
    QRectF boundingRect() const;

    int update(KisWetMap *wetMap);

private:
    const float alpha = 0.33f;

    QPolygonF m_vertices;
    QList<QPointF> m_velocities;
    int m_life;
    qreal m_roughness;
    qreal m_flow;
    QPointF m_motionBias;

    QTime m_startTime;

    qreal m_initSize;
    KoColor m_initColor;

    int m_fix;
};

#endif
