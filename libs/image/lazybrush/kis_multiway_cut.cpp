/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_multiway_cut.h"

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColor.h>

#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_lazy_fill_tools.h"
#include "kis_sequential_iterator.h"
#include <floodfill/kis_scanline_fill.h>


typedef std::pair<KisPaintDeviceSP, KoColor> Scribble;

struct KisMultiwayCut::Private
{
    KisPaintDeviceSP src;
    KisPaintDeviceSP dst;
    KisPaintDeviceSP mask;

    QVector<Scribble> scribbles;

    static void maskOutScribble(KisPaintDeviceSP scribble, KisPaintDeviceSP mask);
};

KisMultiwayCut::KisMultiwayCut(KisPaintDeviceSP src,
                               KisPaintDeviceSP dst)
    : m_d(new Private)
{
    m_d->src = src;
    m_d->dst = dst;
    m_d->mask = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
}

KisMultiwayCut::~KisMultiwayCut()
{
}

void KisMultiwayCut::addScribble(KisPaintDeviceSP dev, const KoColor &color)
{
    m_d->scribbles << Scribble(dev, color);
}


void KisMultiwayCut::Private::maskOutScribble(KisPaintDeviceSP scribble, KisPaintDeviceSP mask)
{
    KIS_ASSERT_RECOVER_RETURN(scribble->pixelSize() == 1);
    KIS_ASSERT_RECOVER_RETURN(mask->pixelSize() == 1);

    QRegion region = scribble->region() & mask->exactBounds();

    Q_FOREACH (const QRect &rc, region.rects()) {
        KisSequentialIterator dstIt(scribble, rc);
        KisSequentialConstIterator mskIt(mask, rc);

        do {
            if (*mskIt.rawDataConst() > 0) {
                *dstIt.rawData() = 0;
            }
        } while (dstIt.nextPixel() && mskIt.nextPixel());
    }
}

void KisMultiwayCut::run()
{
    KisPaintDeviceSP other = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());

    while (m_d->scribbles.size() > 1) {
        Scribble current = m_d->scribbles.takeFirst();
        KisPainter gc(other);

        Q_FOREACH (const Scribble &s, m_d->scribbles) {
            const QRect rc = s.first->extent();
            gc.bitBlt(rc.topLeft(), s.first, rc);
        }

        KisLazyFillTools::cutOneWay(current.second,
                                    m_d->src,
                                    current.first,
                                    other,
                                    m_d->dst,
                                    m_d->mask);

        other->clear();
    }

    // TODO: make configurable!
    const QRect boundingRect = m_d->src->exactBounds();

    // TODO: check if one can use the last cut for this purpose!

    if (m_d->scribbles.size() == 1) {
        Scribble current = m_d->scribbles.takeLast();

        m_d->maskOutScribble(current.first, m_d->mask);

        QVector<QPoint> points =
            KisLazyFillTools::splitIntoConnectedComponents(current.first);

        Q_FOREACH (const QPoint &pt, points) {
            KisScanlineFill fill(m_d->mask, pt, boundingRect);
            fill.fillColor(current.second, m_d->dst);
        }
    }
}
