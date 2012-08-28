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
    KisConvolutionWorkerSpatial(KisPainter *painter, KoUpdater *progress) : KisConvolutionWorker<_IteratorFactory_>(painter, progress) {
        m_pixelPtrCache = 0;
        m_pixelPtrCacheCopy = 0;
        m_minClamp = 0;
        m_maxClamp = 0;
        m_absoluteOffset = 0;
    }

    ~KisConvolutionWorkerSpatial() {
    }

    virtual void execute(const KisConvolutionKernelSP kernel, const KisPaintDeviceSP src, QPoint srcPos, QPoint dstPos, QSize areaSize, const QRect& dataRect) {
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
            QRect r = this->m_painter->selection()->selectedRect().intersect(QRect(srcPos, areaSize));
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

        bool hasProgressUpdater = this->m_progress;
        if (hasProgressUpdater)
            this->m_progress->setProgress(0);

        // Iterate over all pixels in our rect, create a cache of pixels around the current pixel and convolve them.
        m_pixelPtrCache = new double*[m_cacheSize];
        m_pixelPtrCacheCopy = new double*[m_cacheSize];
        for (quint32 c = 0; c < m_cacheSize; ++c) {
            m_pixelPtrCache[c] = new double[channelCount];
            m_pixelPtrCacheCopy[c] = new double[channelCount];
        }

        // decide caching strategy
        enum TraversingDirection { Horizontal, Vertical };
        TraversingDirection traversingDirection = Vertical;
        if (m_kw > m_kh) {
            traversingDirection = Horizontal;
        }

        KisMathToolbox* mathToolbox = KisMathToolboxRegistry::instance()->value(src->colorSpace()->mathToolboxId().id());
        m_toDoubleFuncPtr = QVector<PtrToDouble>(m_convolveChannelsNo);
        if (!mathToolbox->getToDoubleChannelPtr(m_convChannelList, m_toDoubleFuncPtr))
            return;

        m_fromDoubleFuncPtr = QVector<PtrFromDouble>(m_convolveChannelsNo);
        if (!mathToolbox->getFromDoubleChannelPtr(m_convChannelList, m_fromDoubleFuncPtr))
            return;

        qint32 row = srcPos.y();
        qint32 col = srcPos.x();

        // populate pixelPtrCacheCopy for starting position (0, 0)
        qint32 i = 0;
        typename _IteratorFactory_::HLineIterator hitInitSrc = _IteratorFactory_::createHLineIterator(src, col - m_khalfWidth, row - m_khalfHeight, m_kw, dataRect);
        for (quint32 krow = 0; krow < m_kh; ++krow) {
            do {
                const quint8* data = hitInitSrc->rawData();
                for (quint32 k = 0; k < m_convolveChannelsNo; ++k) {
                    const quint32 channelPos = m_convChannelList[k]->pos();
                    m_pixelPtrCacheCopy[i][k] = m_toDoubleFuncPtr[k](data, channelPos);
                }
                ++i;
            } while (hitInitSrc->nextPixel());
            hitInitSrc->nextRow();
        }

        m_kernelFactor = kernel->factor() ? 1.0 / kernel->factor() : 1;
        m_maxClamp = new double[m_convChannelList.count()];
        m_minClamp = new double[m_convChannelList.count()];
        m_absoluteOffset = new double[m_convChannelList.count()];
        for (quint16 i = 0; i < m_convChannelList.count(); ++i) {
            m_minClamp[i] = mathToolbox->minChannelValue(m_convChannelList[i]);
            m_maxClamp[i] = mathToolbox->maxChannelValue(m_convChannelList[i]);
            m_absoluteOffset[i] = (m_maxClamp[i] - m_minClamp[i]) * kernel->offset();
        }

        if (traversingDirection == Horizontal) {
            if(hasProgressUpdater) {
                this->m_progress->setRange(0, areaSize.height());
            }
            typename _IteratorFactory_::HLineIterator hitDst = _IteratorFactory_::createHLineIterator(this->m_painter->device(), dstPos.x(), dstPos.y(), areaSize.width(), dataRect);
            typename _IteratorFactory_::HLineIterator hitSrc = _IteratorFactory_::createHLineIterator(src, srcPos.x(), srcPos.y(), areaSize.width(), dataRect);

            typename _IteratorFactory_::HLineIterator khitSrc = _IteratorFactory_::createHLineIterator(src, col - m_khalfWidth, row + m_khalfHeight, m_kw, dataRect);
            for (int prow = 0; prow < areaSize.height(); ++prow) {
                // reload cache from copy
                for (quint32 i = 0; i < m_cacheSize; ++i)
                    memcpy(m_pixelPtrCache[i], m_pixelPtrCacheCopy[i], channelCount * sizeof(double));

                typename _IteratorFactory_::VLineIterator kitSrc = _IteratorFactory_::createVLineIterator(src, col + m_khalfWidth, row - m_khalfHeight, m_kh, dataRect);
                for (int pcol = 0; pcol < areaSize.width(); ++pcol) {
                    // write original channel values
                    memcpy(hitDst->rawData(), hitSrc->rawData(), m_pixelSize);
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
        } else if (traversingDirection == Vertical) {
            if(hasProgressUpdater) {
                this->m_progress->setRange(0, areaSize.width());
            }
            typename _IteratorFactory_::VLineIterator vitDst = _IteratorFactory_::createVLineIterator(this->m_painter->device(), dstPos.x(), dstPos.y(), areaSize.height(), dataRect);
            typename _IteratorFactory_::VLineIterator vitSrc = _IteratorFactory_::createVLineIterator(src, srcPos.x(), srcPos.y(), areaSize.height(), dataRect);

            typename _IteratorFactory_::VLineIterator kitSrc = _IteratorFactory_::createVLineIterator(src, col + m_khalfWidth, row - m_khalfHeight, m_kh, dataRect);
            for (int pcol = 0; pcol < areaSize.width(); pcol++) {
                // reload cache from copy
                for (quint32 i = 0; i < m_cacheSize; ++i)
                    memcpy(m_pixelPtrCache[i], m_pixelPtrCacheCopy[i], channelCount * sizeof(double));

                typename _IteratorFactory_::HLineIterator khitSrc = _IteratorFactory_::createHLineIterator(src, col - m_khalfWidth, row + m_khalfHeight, m_kw, dataRect);
                for (int prow = 0; prow < areaSize.height(); prow++) {
                    // write original channel values
                    memcpy(vitDst->rawData(), vitSrc->rawData(), m_pixelSize);
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

    inline void convolveCache(quint8* dstPtr) {
        for (quint32 k = 0; k < m_convolveChannelsNo; ++k) {
            qreal interimConvoResult = 0;

            for (quint32 pIndex = 0; pIndex < m_cacheSize; ++pIndex) {
                qreal cacheValue = m_pixelPtrCache[pIndex][k];
                interimConvoResult += m_kernelData[m_cacheSize - pIndex - 1] * cacheValue;
            }

            double channelPixelValue = (double)(interimConvoResult * m_kernelFactor + m_absoluteOffset[k]);

            // clamp values
            if (channelPixelValue > m_maxClamp[k])
                channelPixelValue = m_maxClamp[k];
            else if (channelPixelValue < m_minClamp[k])
                channelPixelValue = m_minClamp[k];

            const quint32 channelPos = m_convChannelList[k]->pos();
            m_fromDoubleFuncPtr[k](dstPtr, channelPos, channelPixelValue);
        }
    }

    inline void moveKernelRight(typename _IteratorFactory_::VLineIterator& kitSrc, double **pixelPtrCache) {
        double** d = pixelPtrCache;

        for (quint32 krow = 0; krow < m_kh; ++krow) {
            double* first = *d;
            memmove(d, d + 1, (m_kw - 1) * sizeof(double *));
            *(d + m_kw - 1) = first;
            d += m_kw;
        }

        qint32 i = m_kw - 1;
        do {
            for (quint32 k = 0; k < m_convolveChannelsNo; ++k) {
                const quint32 channelPos = m_convChannelList[k]->pos();
                pixelPtrCache[i][k] = m_toDoubleFuncPtr[k](kitSrc->rawData(), channelPos);
            }
            i += m_kw;
        } while (kitSrc->nextPixel());
    }

    inline void moveKernelDown(typename _IteratorFactory_::HLineIterator& kitSrc, double **pixelPtrCache) {
        quint8 **tmp = new quint8*[m_kw];
        memcpy(tmp, pixelPtrCache, m_kw * sizeof(double *));
        memmove(pixelPtrCache, pixelPtrCache + m_kw, (m_kw * m_kh - m_kw) * sizeof(quint8 *));
        memcpy(pixelPtrCache + m_kw *(m_kh - 1), tmp, m_kw * sizeof(quint8 *));
        delete[] tmp;

        qint32 i = m_kw * (m_kh - 1);
        do {
            for (quint32 k = 0; k < m_convolveChannelsNo; ++k) {
                const quint32 channelPos = m_convChannelList[k]->pos();
                pixelPtrCache[i][k] = m_toDoubleFuncPtr[k](kitSrc->rawData(), channelPos);
            }
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

    qreal *m_kernelData;
    double** m_pixelPtrCache, ** m_pixelPtrCacheCopy;
    double* m_minClamp, *m_maxClamp, *m_absoluteOffset;

    qreal m_kernelFactor;
    QList<KoChannelInfo *> m_convChannelList;
    QVector<PtrToDouble> m_toDoubleFuncPtr;
    QVector<PtrFromDouble> m_fromDoubleFuncPtr;
};


#endif
