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

#include "kis_colorize_stroke_strategy.h"

#include <QBitArray>

#include "kis_multiway_cut.h"
#include "kis_paint_device.h"
#include "kis_lazy_fill_tools.h"
#include "kis_gaussian_kernel.h"
#include "kis_painter.h"
#include "kis_default_bounds_base.h"
#include "kis_lod_transform.h"
#include "kis_node.h"
#include "kis_image_config.h"

using namespace KisLazyFillTools;

struct KisColorizeStrokeStrategy::Private
{
    Private() : filteredSourceValid(false) {}
    Private(const Private &rhs)
        : src(rhs.src),
          dst(rhs.dst),
          filteredSource(rhs.filteredSource),
          internalFilteredSource(rhs.internalFilteredSource),
          filteredSourceValid(rhs.filteredSourceValid),
          boundingRect(rhs.boundingRect),
          keyStrokes(rhs.keyStrokes),
          dirtyNode(rhs.dirtyNode)
    {}

    KisPaintDeviceSP src;
    KisPaintDeviceSP dst;
    KisPaintDeviceSP filteredSource;
    KisPaintDeviceSP internalFilteredSource;
    bool filteredSourceValid;
    QRect boundingRect;

    QVector<KeyStroke> keyStrokes;
    KisNodeSP dirtyNode;
};

KisColorizeStrokeStrategy::KisColorizeStrokeStrategy(KisPaintDeviceSP src,
                                                     KisPaintDeviceSP dst,
                                                     KisPaintDeviceSP filteredSource,
                                                     bool filteredSourceValid,
                                                     const QRect &boundingRect,
                                                     KisNodeSP dirtyNode)
    : m_d(new Private)
{
    m_d->src = src;
    m_d->dst = dst;
    m_d->filteredSource = filteredSource;
    m_d->boundingRect = boundingRect;
    m_d->filteredSourceValid = filteredSourceValid;
    m_d->dirtyNode = dirtyNode;

    enableJob(JOB_INIT, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
}

KisColorizeStrokeStrategy::KisColorizeStrokeStrategy(const KisColorizeStrokeStrategy &rhs, int levelOfDetail)
    : KisSimpleStrokeStrategy(rhs),
      m_d(new Private(*rhs.m_d))
{
    KisLodTransform t(levelOfDetail);
    m_d->boundingRect = t.map(rhs.m_d->boundingRect);
}

KisColorizeStrokeStrategy::~KisColorizeStrokeStrategy()
{
}

void KisColorizeStrokeStrategy::addKeyStroke(KisPaintDeviceSP dev, const KoColor &color)
{
    KoColor convertedColor(color);
    convertedColor.convertTo(m_d->dst->colorSpace());

    m_d->keyStrokes << KeyStroke(dev, convertedColor);
}

void KisColorizeStrokeStrategy::initStrokeCallback()
{
    if (!m_d->filteredSourceValid) {
        KisPaintDeviceSP filteredMainDev = KisPainter::convertToAlphaAsAlpha(m_d->src);

        // optional filtering
        // KisGaussianKernel::applyLoG(filteredMainDev,
        //                             m_d->boundingRect,
        //                             1.0,
        //                             QBitArray(), 0);

        normalizeAndInvertAlpha8Device(filteredMainDev, m_d->boundingRect);

        KisDefaultBoundsBaseSP oldBounds = m_d->filteredSource->defaultBounds();
        m_d->filteredSource->makeCloneFrom(filteredMainDev, m_d->boundingRect);
        m_d->filteredSource->setDefaultBounds(oldBounds);
    }

    KisMultiwayCut cut(m_d->filteredSource, m_d->dst, m_d->boundingRect);

    Q_FOREACH (const KeyStroke &stroke, m_d->keyStrokes) {
        cut.addKeyStroke(new KisPaintDevice(*stroke.dev), stroke.color);
    }

    cut.run();

    m_d->dirtyNode->setDirty(m_d->boundingRect);
    emit sigFinished();
}

KisStrokeStrategy* KisColorizeStrokeStrategy::createLodClone(int levelOfDetail)
{
    KisImageConfig cfg;
    if (!cfg.useLodForColorizeMask()) return 0;

    KisColorizeStrokeStrategy *clone = new KisColorizeStrokeStrategy(*this, levelOfDetail);
    return clone;
}
