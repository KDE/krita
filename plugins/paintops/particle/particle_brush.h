/*
 *  SPDX-FileCopyrightText: 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef _PARTICLE_BRUSH_H_
#define _PARTICLE_BRUSH_H_

#include "kis_paint_device.h"
#include "kis_debug.h"
#include <QPointF>


class KisParticleBrushProperties
{
public:
    quint16 particleCount;
    quint16 iterations;
    qreal weight;
    qreal gravity;
    QPointF scale;
};

class KisRandomAccessor;
class KoColorSpace;
class KoColor;

class ParticleBrush
{

public:

    ParticleBrush();
    ~ParticleBrush();
    void initParticles();
    void draw(KisPaintDeviceSP dab, const KoColor& color, const QPointF &pos);

    void setInitialPosition(const QPointF &pos);
    void setProperties(KisParticleBrushProperties * properties) {
        m_properties = properties;
    }

private:
    /// paints wu particle, similar to spray version but you can turn on respecting opacity of the tool and add weight to opacity
    /// also the particle respects opacity in the destination pixel buffer
    void paintParticle(KisRandomAccessorSP writeAccessor, const KoColorSpace *cs,const QPointF &pos, const KoColor& color, qreal weight, bool respectOpacity);

    QVector<QPointF> m_particlePos;
    QVector<QPointF> m_particleNextPos;
    QVector<qreal> m_accelaration;

    KisParticleBrushProperties * m_properties;
};

#endif
