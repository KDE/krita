/*
 *  Copyright (c) 2005, 2008, 2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2009, 2010 Edward Apap <schumifer@hotmail.com>
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

#ifndef KIS_CONVOLUTION_WORKER_SPATIAL_H
#define KIS_CONVOLUTION_WORKER_SPATIAL_H

#include "kis_convolution_worker.h"
#include "kis_math_toolbox.h"

template <class _IteratorFactory_>
class KisConvolutionWorkerSpatial : public KisConvolutionWorker<_IteratorFactory_>
{
public:
    KisConvolutionWorkerSpatial(KisPainter *painter, KoUpdater *progress)
        : KisConvolutionWorker<_IteratorFactory_>(painter, progress)
        ,  m_alphaCachePos(-1)
        ,  m_alphaRealPos(-1)
        ,  m_pixelPtrCache(0)
        ,  m_pixelPtrCacheCopy(0)
        ,  m_minClamp(0)
        ,  m_maxClamp(0)
        ,  m_absoluteOffset(0)
    {
    }

    ~KisConvolutionWorkerSpatial() override {
    }

    inline void loadPixelToCache(qreal **cache, const quint8 *data, int index) {
        // no alpha is rare case, so just multiply by 1.0 in that case
        qreal alphaValue = m_alphaRealPos >= 0 ?
            m_toDoubleFuncPtr[m_alphaCachePos](data, m_alphaRealPos) : 1.0;

        for (quint32 k = 0; k < m_convolveChannelsNo; ++k) {
            if (k != (quint32)m_alphaCachePos) {
                const quint32 channelPos = m_convChannelList[k]->pos();
                cache[index][k] = m_toDoubleFuncPtr[k](data, channelPos) * alphaValue;
            } else {
                cache[index][k] = alphaValue;
            }
        }

    }

    void execute(const KisConvolutionKernelSP kernel, const KisPaintDeviceSP src, QPoint srcPos, QPoint dstPos, QSize areaSize, const QRect& dataRect) override {
        // store some kernel characteristics
        m_kw = kernel->width();
        m_kh = kernel->height();
        m_khalfWidth = (m_kw - 1) / 2;
        m_khalfHeight = (m_kh - 1) / 2;
        m_cacheSize = m_kw * m_kh;
        m_pixelSize = src->colorSpace()->pixelSize();
        quint32 channelCount = src->colorSpace()->channelCount();

        m_kernelData = new qreal[m_cacheSize];
        qreal *kernelDataPtr = m_kernelData;

        // fill in data
        for (quint32 r = 0; r < kernel->height(); r++) {
            for (quint32 c = 0; c < kernel->width(); c++) {
                *kernelDataPtr = (*(kernel->data()))(r, c);
                kernelDataPtr++;
            }
        }

        // Make the area we cover as small as possible
        if (this->m_painter->selection()) {
            QRect r = this->m_painter->selection()->selectedRect().intersected(QRect(srcPos, areaSize));
            dstPos += r.topLeft() - srcPos;
            srcPos = r.topLeft();
            areaSize = r.size();
        }

        if (areaSize.width() == 0 || areaSize.height() == 0)
            return;

        // Don't convolve with an even sized kernel
        Q_ASSERT((m_kw & 0x01) == 1 || (m_kh & 0x01) == 1 || kernel->factor() != 0);

        // find out which channels need be convolved
        m_convChannelList = this->convolvableChannelList(src);
        m_convolveChannelsNo = m_convChannelList.count();

        for (int i = 0; i < m_convChannelList.size(); i++) {
            if (m_convChannelList[i]->channelType() == KoChannelInfo::ALPHA) {
                m_alphaCachePos = i;
                m_alphaRealPos = m_convChannelList[i]->pos();
            }
        }

        bool hasProgressUpdater = this->m_progress;
        if (hasProgressUpdater)
            this->m_progress->setProgress(0);

        // Iterate over all pixels in our rect, create a cache of pixels around the current pixel and convolve them.
        m_pixelPtrCache = new qreal*[m_cacheSize];
        m_pixelPtrCacheCopy = new qreal*[m_cacheSize];
        for (quint32 c = 0; c < m_cacheSize; ++c) {
            m_pixelPtrCache[c] = new qreal[channelCount];
            m_pixelPtrCacheCopy[c] = new qreal[channelCount];
        }

        // decide caching strategy
        enum TraversingDirection { Horizontal, Vertical };
        TraversingDirection traversingDirection = Vertical;
        if (m_kw > m_kh) {
            traversingDirection = Horizontal;
        }

        KisMathToolbox mathToolbox;
        m_toDoubleFuncPtr = QVector<PtrToDouble>(m_convolveChannelsNo);
        if (!mathToolbox.getToDoubleChannelPtr(m_convChannelList, m_toDoubleFuncPtr))
            return;

        m_fromDoubleFuncPtr = QVector<PtrFromDouble>(m_convolveChannelsNo);
        if (!mathToolbox.getFromDoubleChannelPtr(m_convChannelList, m_fromDoubleFuncPtr))
            return;

        m_kernelFactor = kernel->factor() ? 1.0 / kernel->factor() : 1;
        m_maxClamp = new qreal[m_convChannelList.count()];
        m_minClamp = new qreal[m_convChannelList.count()];
        m_absoluteOffset = new qreal[m_convChannelList.count()];
        for (quint16 i = 0; i < m_convChannelList.count(); ++i) {
            m_minClamp[i] = mathToolbox.minChannelValue(m_convChannelList[i]);
            m_maxClamp[i] = mathToolbox.maxChannelValue(m_convChannelList[i]);
            m_absoluteOffset[i] = (m_maxClamp[i] - m_minClamp[i]) * kernel->offset();
        }

        qint32 row = srcPos.y();
        qint32 col = srcPos.x();

        // populate pixelPtrCacheCopy for starting position (0, 0)
        qint32 i = 0;
        typename _IteratorFactory_::HLineConstIterator hitInitSrc = _IteratorFactory_::createHLineConstIterator(src, col - m_khalfWidth, row - m_khalfHeight, m_kw, dataRect);

        for (quint32 krow = 0; krow < m_kh; ++krow) {
            do {
                const quint8* data = hitInitSrc->oldRawData();
                loadPixelToCache(m_pixelPtrCacheCopy, data, i);
                ++i;
            } while (hitInitSrc->nextPixel());
            hitInitSrc->nextRow();
        }


        if (traversingDirection == Horizontal) {
            if(hasProgressUpdater) {
                this->m_progress->setRange(0, areaSize.height());
            }
            typename _IteratorFactory_::HLineIterator hitDst = _IteratorFactory_::createHLineIterator(this->m_painter->device(), dstPos.x(), dstPos.y(), areaSize.width(), dataRect);
            typename _IteratorFactory_::HLineConstIterator hitSrc = _IteratorFactory_::createHLineConstIterator(src, srcPos.x(), srcPos.y(), areaSize.width(), dataRect);

            typename _IteratorFactory_::HLineConstIterator khitSrc = _IteratorFactory_::createHLineConstIterator(src, col - m_khalfWidth, row + m_khalfHeight, m_kw, dataRect);
            for (int prow = 0; prow < areaSize.height(); ++prow) {
                // reload cache from copy
                for (quint32 i = 0; i < m_cacheSize; ++i)
                    memcpy(m_pixelPtrCache[i], m_pixelPtrCacheCopy[i], channelCount * sizeof(qreal));

                typename _IteratorFactory_::VLineConstIterator kitSrc = _IteratorFactory_::createVLineConstIterator(src, col + m_khalfWidth, row - m_khalfHeight, m_kh, dataRect);
                for (int pcol = 0; pcol < areaSize.width(); ++pcol) {
                    // write original channel values
                    memcpy(hitDst->rawData(), hitSrc->oldRawData(), m_pixelSize);
                    convolveCache(hitDst->rawData());

                    ++col;
                    kitSrc->nextColumn();
                    hitDst->nextPixel();
                    hitSrc->nextPixel();
                    moveKernelRight(kitSrc, m_pixelPtrCache);
                }

                row++;
                khitSrc->nextRow();
                hitDst->nextRow();
                hitSrc->nextRow();
                col = srcPos.x();

                moveKernelDown(khitSrc, m_pixelPtrCacheCopy);

                if (hasProgressUpdater) {
                    this->m_progress->setValue(prow);

                    if (this->m_progress->interrupted()) {
                        cleanUp();
                        return;
                    }
                }

            }
        } else /* if (traversingDirection == Vertical) */ {
            if(hasProgressUpdater) {
                this->m_progress->setRange(0, areaSize.width());
            }
            typename _IteratorFactory_::VLineIterator vitDst = _IteratorFactory_::createVLineIterator(this->m_painter->device(), dstPos.x(), dstPos.y(), areaSize.height(), dataRect);
            typename _IteratorFactory_::VLineConstIterator vitSrc = _IteratorFactory_::createVLineConstIterator(src, srcPos.x(), srcPos.y(), areaSize.height(), dataRect);

            typename _IteratorFactory_::VLineConstIterator kitSrc = _IteratorFactory_::createVLineConstIterator(src, col + m_khalfWidth, row - m_khalfHeight, m_kh, dataRect);
            for (int pcol = 0; pcol < areaSize.width(); pcol++) {
                // reload cache from copy
                for (quint32 i = 0; i < m_cacheSize; ++i)
                    memcpy(m_pixelPtrCache[i], m_pixelPtrCacheCopy[i], channelCount * sizeof(qreal));

                typename _IteratorFactory_::HLineConstIterator khitSrc = _IteratorFactory_::createHLineConstIterator(src, col - m_khalfWidth, row + m_khalfHeight, m_kw, dataRect);
                for (int prow = 0; prow < areaSize.height(); prow++) {
                    // write original channel values
                    memcpy(vitDst->rawData(), vitSrc->oldRawData(), m_pixelSize);
                    convolveCache(vitDst->rawData());

                    ++row;
                    khitSrc->nextRow();
                    vitDst->nextPixel();
                    vitSrc->nextPixel();
                    moveKernelDown(khitSrc, m_pixelPtrCache);
                }

                ++col;
                kitSrc->nextColumn();
                vitDst->nextColumn();
                vitSrc->nextColumn();
                row = srcPos.y();

                moveKernelRight(kitSrc, m_pixelPtrCacheCopy);

                if (hasProgressUpdater) {
                    this->m_progress->setValue(pcol);

                    if (this->m_progress->interrupted()) {
                        cleanUp();
                        return;
                    }
                }
            }
        }
        cleanUp();
    }

    inline void limitValue(qreal *value, qreal lowBound, qreal highBound) {
        if (*value > highBound) {
            *value = highBound;
        } else if (!(*value >= lowBound)) {  // value < lowBound or value == NaN
            // IEEE compliant comparisons with NaN are always false
            *value = lowBound;
        }
    }

    template <bool additionalMultiplierActive>
    inline qreal convolveOneChannelFromCache(quint8* dstPtr, quint32 channel, qreal additionalMultiplier = 0.0) {
        qreal interimConvoResult = 0;

        for (quint32 pIndex = 0; pIndex < m_cacheSize; ++pIndex) {
            qreal cacheValue = m_pixelPtrCache[pIndex][channel];
            interimConvoResult += m_kernelData[m_cacheSize - pIndex - 1] * cacheValue;
        }

        qreal channelPixelValue;
        if (additionalMultiplierActive) {
            channelPixelValue = (interimConvoResult * m_kernelFactor) * additionalMultiplier + m_absoluteOffset[channel];
        } else {
            channelPixelValue = interimConvoResult * m_kernelFactor + m_absoluteOffset[channel];
        }

        limitValue(&channelPixelValue, m_minClamp[channel], m_maxClamp[channel]);

        const quint32 channelPos = m_convChannelList[channel]->pos();
        m_fromDoubleFuncPtr[channel](dstPtr, channelPos, channelPixelValue);

        return channelPixelValue;
    }

    inline void convolveCache(quint8* dstPtr) {
        if (m_alphaCachePos >= 0) {
            qreal alphaValue = convolveOneChannelFromCache<false>(dstPtr, m_alphaCachePos);

            // TODO: we need a special case for applying LoG filter,
            // when the alpha i suniform and therefore should not be
            // filtered!
            //alphaValue = 255.0;

            if (alphaValue != 0.0) {
                qreal alphaValueInv = 1.0 / alphaValue;

                for (quint32 k = 0; k < m_convolveChannelsNo; ++k) {
                    if (k == (quint32)m_alphaCachePos) continue;
                    convolveOneChannelFromCache<true>(dstPtr, k, alphaValueInv);
                }
            } else {
                for (quint32 k = 0; k < m_convolveChannelsNo; ++k) {
                    if (k == (quint32)m_alphaCachePos) continue;

                    const qreal zeroValue = 0.0;
                    const quint32 channelPos = m_convChannelList[k]->pos();
                    m_fromDoubleFuncPtr[k](dstPtr, channelPos, zeroValue);
                }
            }
        } else {
            for (quint32 k = 0; k < m_convolveChannelsNo; ++k) {
                convolveOneChannelFromCache<false>(dstPtr, k);
            }
        }
    }

    inline void moveKernelRight(typename _IteratorFactory_::VLineConstIterator& kitSrc, qreal **pixelPtrCache) {
        qreal** d = pixelPtrCache;

        for (quint32 krow = 0; krow < m_kh; ++krow) {
            qreal* first = *d;
            memmove(d, d + 1, (m_kw - 1) * sizeof(qreal *));
            *(d + m_kw - 1) = first;
            d += m_kw;
        }

        qint32 i = m_kw - 1;
        do {
            const quint8* data = kitSrc->oldRawData();
            loadPixelToCache(pixelPtrCache, data, i);
            i += m_kw;
        } while (kitSrc->nextPixel());
    }

    inline void moveKernelDown(typename _IteratorFactory_::HLineConstIterator& kitSrc, qreal **pixelPtrCache) {
        quint8 **tmp = new quint8*[m_kw];
        memcpy(tmp, pixelPtrCache, m_kw * sizeof(qreal *));
        memmove(pixelPtrCache, pixelPtrCache + m_kw, (m_kw * m_kh - m_kw) * sizeof(quint8 *));
        memcpy(pixelPtrCache + m_kw *(m_kh - 1), tmp, m_kw * sizeof(quint8 *));
        delete[] tmp;

        qint32 i = m_kw * (m_kh - 1);
        do {
            const quint8* data = kitSrc->oldRawData();
            loadPixelToCache(pixelPtrCache, data, i);
            i++;
        } while (kitSrc->nextPixel());
    }

    void cleanUp() {
        for (quint32 c = 0; c < m_cacheSize; ++c) {
            delete[] m_pixelPtrCache[c];
            delete[] m_pixelPtrCacheCopy[c];
        }

        delete[] m_kernelData;
        delete[] m_pixelPtrCache;
        delete[] m_pixelPtrCacheCopy;

        delete[] m_minClamp;
        delete[] m_maxClamp;
        delete[] m_absoluteOffset;
    }

private:
    quint32 m_kw, m_kh;
    quint32 m_khalfWidth, m_khalfHeight;
    quint32 m_convolveChannelsNo;
    quint32 m_cacheSize, m_pixelSize;

    int m_alphaCachePos;
    int m_alphaRealPos;

    qreal *m_kernelData;
    qreal** m_pixelPtrCache, ** m_pixelPtrCacheCopy;
    qreal* m_minClamp, *m_maxClamp, *m_absoluteOffset;

    qreal m_kernelFactor;
    QList<KoChannelInfo *> m_convChannelList;
    QVector<PtrToDouble> m_toDoubleFuncPtr;
    QVector<PtrFromDouble> m_fromDoubleFuncPtr;
};


#endif
