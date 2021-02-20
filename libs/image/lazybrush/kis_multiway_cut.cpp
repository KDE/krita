/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_multiway_cut.h"

#include <KoColorSpaceRegistry.h>
#include <KoColorSpace.h>
#include <KoColor.h>

#include "KisRegion.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_lazy_fill_tools.h"
#include "kis_sequential_iterator.h"
#include <floodfill/kis_scanline_fill.h>


using namespace KisLazyFillTools;

struct KisMultiwayCut::Private
{
    KisPaintDeviceSP src;
    KisPaintDeviceSP dst;
    KisPaintDeviceSP mask;
    QRect boundingRect;

    QVector<KeyStroke> keyStrokes;

    static void maskOutKeyStroke(KisPaintDeviceSP keyStrokeDevice, KisPaintDeviceSP mask, const QRect &boundingRect);
};

KisMultiwayCut::KisMultiwayCut(KisPaintDeviceSP src,
                               KisPaintDeviceSP dst,
                               const QRect &boundingRect)
    : m_d(new Private)
{
    m_d->src = src;
    m_d->dst = dst;
    m_d->mask = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    m_d->boundingRect = boundingRect;
}

KisMultiwayCut::~KisMultiwayCut()
{
}

void KisMultiwayCut::addKeyStroke(KisPaintDeviceSP dev, const KoColor &color)
{
    m_d->keyStrokes << KeyStroke(dev, color);
}


void KisMultiwayCut::Private::maskOutKeyStroke(KisPaintDeviceSP keyStrokeDevice, KisPaintDeviceSP mask, const QRect &boundingRect)
{
    KIS_ASSERT_RECOVER_RETURN(keyStrokeDevice->pixelSize() == 1);
    KIS_ASSERT_RECOVER_RETURN(mask->pixelSize() == 1);

    KisRegion region =
        keyStrokeDevice->region() &
        mask->exactBounds() & boundingRect;

    Q_FOREACH (const QRect &rc, region.rects()) {
        KisSequentialIterator dstIt(keyStrokeDevice, rc);
        KisSequentialConstIterator mskIt(mask, rc);

        while (dstIt.nextPixel() && mskIt.nextPixel()) {
            if (*mskIt.rawDataConst() > 0) {
                *dstIt.rawData() = 0;
            }
        }
    }
}

bool keyStrokesOrder(const KeyStroke &a, const KeyStroke &b)
{
    const bool aTransparent = a.color.opacityU8() == OPACITY_TRANSPARENT_U8;
    const bool bTransparent = b.color.opacityU8() == OPACITY_TRANSPARENT_U8;

    if (aTransparent && !bTransparent) return true;
    if (!aTransparent && bTransparent) return false;

    const QRect aRect = a.dev->extent();
    const QRect bRect = b.dev->extent();

    const int aArea = aRect.width() * aRect.height();
    const int bArea = bRect.width() * bRect.height();

    return aArea > bArea;
}

void KisMultiwayCut::run()
{
    KisPaintDeviceSP other(new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8()));

    /**
     * First sort all the key strokes in a way that all the
     * transparent strokes go to the beginning of the list.
     *
     * This is juat an heuristic: the transparent stroke usually
     * represents the background so it is the bigger one. And since
     * our algorithm is greedy, we should cover the biggest area
     * as fast as possible.
     */

    std::stable_sort(m_d->keyStrokes.begin(), m_d->keyStrokes.end(), keyStrokesOrder);

    while (m_d->keyStrokes.size() > 1) {
        KeyStroke current = m_d->keyStrokes.takeFirst();

        // if current scribble is empty, it just has no effect
        if (current.dev->exactBounds().isEmpty()) continue;

        KisPainter gc(other);

        Q_FOREACH (const KeyStroke &s, m_d->keyStrokes) {
            const QRect rc = s.dev->extent() & m_d->boundingRect;
            gc.bitBlt(rc.topLeft(), s.dev, rc);
        }

        // if other is empty, it means that *all* other strokes are
        // empty, so there is no reason to continue the process
        if (other->exactBounds().isEmpty()) {
            m_d->keyStrokes.clear();
            m_d->keyStrokes << current;
            break;
        }

        KisLazyFillTools::cutOneWay(current.color,
                                    m_d->src,
                                    current.dev,
                                    other,
                                    m_d->dst,
                                    m_d->mask,
                                    m_d->boundingRect);

        other->clear();
    }

    // TODO: check if one can use the last cut for this purpose!

    if (m_d->keyStrokes.size() == 1) {
        KeyStroke current = m_d->keyStrokes.takeLast();

        m_d->maskOutKeyStroke(current.dev, m_d->mask, m_d->boundingRect);

        QVector<QPoint> points =
            KisLazyFillTools::splitIntoConnectedComponents(current.dev, m_d->boundingRect);

        Q_FOREACH (const QPoint &pt, points) {
            KisScanlineFill fill(m_d->mask, pt, m_d->boundingRect);
            fill.fillColor(current.color, m_d->dst);
        }
    }
}

KisPaintDeviceSP KisMultiwayCut::srcDevice() const
{
    return m_d->src;
}

KisPaintDeviceSP KisMultiwayCut::dstDevice() const
{
    return m_d->dst;
}
