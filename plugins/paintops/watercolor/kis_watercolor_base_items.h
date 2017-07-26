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

#ifndef KIS_WATERCOLOR_BASE_ITEMS_H
#define KIS_WATERCOLOR_BASE_ITEMS_H

#include <KoRTree.h>
#include "kis_splat.h"
#include "kis_wetmap.h"
#include <QTime>
#include <QObject>
#include <QList>
#include "kis_painter.h"

class KisWatercolorBaseItems : public QObject
{
    Q_OBJECT

public:
    void update();

public:
    static KisWatercolorBaseItems* instance() {
        static KisWatercolorBaseItems instance;
        return &instance;
    }

    void paint(QPointF pos, qreal radius, int brushType, const KoColor &color);

    void repaint(KisPainter *painter);
    void drySystem();

private:
    static KisWatercolorBaseItems * p_instance;
    KisWatercolorBaseItems();
    KisWatercolorBaseItems( const KisWatercolorBaseItems& );
    KisWatercolorBaseItems& operator=( KisWatercolorBaseItems& );

    KisWetMap *m_wetMap;
    QList<KisSplat *> m_flowing;
    KoRTree<KisSplat *> m_fixed;
    QList<KisSplat *> m_dried;
};

#endif // KIS_WATERCOLOR_BASE_ITEMS_H
