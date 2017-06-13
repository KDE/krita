/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_splat.h"
#include "kis_random_generator.h"
#include <cmath>

#define START_OPACITY 50
#define STANDART_LIFETIME 30

KisSplat::KisSplat(QPointF offset, int width, KoColor splatColor)
    : m_life(STANDART_LIFETIME), m_roughness(1.f), m_flow(1.f),
      m_motionBias(QPointF(0.f, 0.f)), m_initColor(splatColor)
{
    m_fix = 8*STANDART_LIFETIME;
    m_initColor.setOpacity(quint8(START_OPACITY));

    int r = width / 2;
    int n = 128;

    qreal dt = 2.f * M_PI / n;
    QPointF p;

    for (int i = 0; i < n; i++)
    {
        p.setX(cos(i * dt));
        p.setY(sin(i * dt));
        m_vertices.push_back(QPointF(static_cast <qreal> (r * p.x()) + offset.x(),
                                     static_cast <qreal> (r * p.y()) + offset.y()));
        m_velocities.push_back(QPointF(2.f * p.x(),
                                       2.f * p.y()));
    }
    m_initSize = CalcSize();
    m_startTime = QTime::currentTime();
}

KisSplat::KisSplat(QPointF offset, QPointF velocityBias, int width, int life,
                   qreal roughness, qreal flow, qreal radialSpeed, KoColor splatColor)
    : m_life(life), m_roughness(roughness), m_flow(flow),
      m_motionBias(velocityBias), m_initColor(splatColor)
{
    m_fix = 8*STANDART_LIFETIME;
    m_initColor.setOpacity(quint8(START_OPACITY));
    int r = width / 2;
    int n = 128;

    qreal dt = 2.f * M_PI / n;
    QPointF p;
    for (int i = 0; i < n; i++)
    {
        p.setX(cos(i * dt));
        p.setY(sin(i * dt));
        m_vertices.push_back(QPointF(static_cast <qreal> (r * p.x()) + offset.x(),
                                     static_cast <qreal> (r * p.y()) + offset.y()));
        m_velocities.push_back(QPointF(radialSpeed * p.x(),
                                       radialSpeed * p.y()));
    }
    m_initSize = CalcSize();
    m_startTime = QTime::currentTime();
}

qreal KisSplat::CalcSize()
{
    if (m_vertices.length() < 3)
        return 0.f;

    QPointF v0 = m_vertices[0];
    qreal v0x = v0.x();
    qreal v0y = v0.y();
    QPointF e0 = m_vertices[1] - v0;
    qreal e0x = e0.x();
    qreal e0y = e0.y();
    qreal s = 0.f;
    int length = m_vertices.length();

    for (int i = 2; i < length; i++) {
        QPointF v2 = m_vertices[i];
        qreal e1x = v2.x() - v0x;
        qreal e1y = v2.y() - v0y;
        s += e0x * e1y - e0y * e1x;
        e0x = e1x;
        e0y = e1y;
    }

    return s >= 0 ? s : -s;
}

KoColor KisSplat::getColor()
{
    KoColor current;
    current = m_initColor;
    qreal multiply = m_initSize / CalcSize();
    if (multiply < 0.f || multiply > 1.f)
        multiply = 1;

    current.setOpacity(current.opacityU8() * multiply);
    return current;
}

QPainterPath KisSplat::shape() const
{
    QPainterPath path;

    int len = m_vertices.length();
    path = *(new QPainterPath(m_vertices[0]));

    for (int i = 0; i < len-2; i+=2) {
        path.quadTo(m_vertices[i+1], m_vertices[i+2]);
    }

    path.quadTo(m_vertices[len-1], m_vertices[0]);

    return path;
}

QRectF KisSplat::boundingRect() const
{
    return m_vertices.boundingRect();
}

int KisSplat::update(KisWetMap *wetMap)
{
    if (m_life <= 0) {
        if (m_fix <= 0) {
            return KisSplat::Dried;
        } else {
            m_fix--;
            return KisSplat::Fixed;
        }
    }

    m_life--;

    KisRandomGenerator randG(0);

    for (int i = 0; i < m_vertices.length(); i++) {
        QPointF x = m_vertices[i];
        QPointF v = m_velocities[i];
        QPointF d = (1.f - alpha) * m_motionBias + alpha / randG.doubleRandomAt(1.f, 1.f + m_roughness) * v;

        QPointF x1 = x + m_flow * d; + QPointF(randG.doubleRandomAt(-m_roughness, m_roughness),
                                               randG.doubleRandomAt(-m_roughness, m_roughness));
        quint16 wet = wetMap->getWater((int)x1.x(), (int)x1.y()); // Считываение количества жидкости
        if (wet > 0)
           m_vertices[i] = x1;
    }

    if (!m_life) {
        for (int i = 0; i < m_vertices.length(); i++) {
            m_velocities[i] = QPointF(0, 0);
        }
    }

    return KisSplat::Flowing;
}
