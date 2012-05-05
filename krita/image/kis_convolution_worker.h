/*
 *  Copyright (c) 2005, 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2010 Edward Apap <schumifer@hotmail.com>
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

#ifndef KIS_CONVOLUTION_WORKER_H
#define KIS_CONVOLUTION_WORKER_H

#include <KoUpdater.h>

#include "kis_paint_device.h"
#include "kis_iterator_ng.h"
#include "kis_repeat_iterators_pixel.h"
#include "kis_painter.h"
#include <QBitArray>

struct StandardIteratorFactory {
    typedef KisHLineIteratorSP HLineIterator;
    typedef KisVLineIteratorSP VLineIterator;
    typedef KisHLineConstIteratorSP HLineConstIterator;
    typedef KisVLineConstIteratorSP VLineConstIterator;
    inline static KisHLineIteratorSP createHLineIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 w, const QRect&) {
        return src->createHLineIteratorNG(x, y, w);
    }
    inline static KisVLineIteratorSP createVLineIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 h, const QRect&) {
        return src->createVLineIteratorNG(x, y, h);
    }
    inline static KisHLineConstIteratorSP createHLineConstIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 w, const QRect&) {
        return src->createHLineConstIteratorNG(x, y, w);
    }
    inline static KisVLineConstIteratorSP createVLineConstIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 h, const QRect&) {
        return src->createVLineConstIteratorNG(x, y, h);
    }
};

struct RepeatIteratorFactory {
    typedef KisHLineIteratorSP HLineIterator;
    typedef KisVLineIteratorSP VLineIterator;
    typedef KisRepeatHLineConstIteratorSP HLineConstIterator;
    typedef KisRepeatVLineConstIteratorSP VLineConstIterator;
    inline static KisHLineIteratorSP createHLineIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 w, const QRect&) {
        return src->createHLineIteratorNG(x, y, w);
    }
    inline static KisVLineIteratorSP createVLineIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 h, const QRect&) {
        return src->createVLineIteratorNG(x, y, h);
    }
    inline static KisRepeatHLineConstIteratorSP createHLineConstIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 w, const QRect& _dataRect) {
        return src->createRepeatHLineConstIterator(x, y, w, _dataRect);
    }
    inline static KisRepeatVLineConstIteratorSP createVLineConstIterator(KisPaintDeviceSP src, qint32 x, qint32 y, qint32 h, const QRect& _dataRect) {
        return src->createRepeatVLineConstIterator(x, y, h, _dataRect);
    }
};

template <class _IteratorFactory_>
class KisConvolutionWorker
{
public:
    KisConvolutionWorker(KisPainter *painter, KoUpdater *progress)
    {
        m_painter = painter;
        m_progress = progress;
    }

    virtual ~KisConvolutionWorker()
    {
    }

    virtual void execute(const KisConvolutionKernelSP kernel, const KisPaintDeviceSP src, QPoint srcPos, QPoint dstPos, QSize areaSize, const QRect& dataRect) = 0;

protected:
    QList<KoChannelInfo *> convolvableChannelList(const KisPaintDeviceSP src)
    {
        QBitArray painterChannelFlags = m_painter->channelFlags();
        if (painterChannelFlags.isEmpty()) {
            painterChannelFlags = QBitArray(src->colorSpace()->channelCount(), true);
        }
        Q_ASSERT(static_cast<quint32>(painterChannelFlags.size()) == src->colorSpace()->channelCount());
        QList<KoChannelInfo *> channelInfo = src->colorSpace()->channels();
        QList<KoChannelInfo *> convChannelList;

        for (qint32 c = 0; c < channelInfo.count(); ++c) {
            if (painterChannelFlags.testBit(c)) {
                convChannelList.append(channelInfo[c]);
            }
        }

        return convChannelList;
    }

protected:
    KisPainter* m_painter;
    KoUpdater* m_progress;
};


#endif
