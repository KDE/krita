/*
 *  Copyright (c) 2010 Edward Apap <schumifer@hotmail.com>
 *  Copyright (c) 2011 José Luis Vergara Toloza <pentalis@gmail.com>
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

#ifndef KIS_CONVOLUTION_WORKER_FFT_H
#define KIS_CONVOLUTION_WORKER_FFT_H

#include <iostream>

#include <KoChannelInfo.h>

#include "kis_convolution_worker.h"
#include "kis_math_toolbox.h"

#include <QMutex>
#include <QVector>
#include <QTextStream>
#include <QFile>
#include <QDir>

#include <fftw3.h>

template<class _IteratorFactory_> class KisConvolutionWorkerFFT;
class KisConvolutionWorkerFFTLock
{
private:
    static QMutex fftwMutex;
    template<class _IteratorFactory_> friend class KisConvolutionWorkerFFT;
};

QMutex KisConvolutionWorkerFFTLock::fftwMutex;


template<class _IteratorFactory_>
class KisConvolutionWorkerFFT : public KisConvolutionWorker<_IteratorFactory_>
{
public:
    KisConvolutionWorkerFFT(KisPainter *painter, KoUpdater *progress)
        : KisConvolutionWorker<_IteratorFactory_>(painter, progress),
          m_currentProgress(0),
          m_kernelFFT(0)
    {
    }

    ~KisConvolutionWorkerFFT()
    {
    }


    virtual void execute(const KisConvolutionKernelSP kernel, const KisPaintDeviceSP src, QPoint srcPos, QPoint dstPos, QSize areaSize, const QRect& dataRect)
    {
        // Make the area we cover as small as possible
        if (this->m_painter->selection())
        {
            QRect r = this->m_painter->selection()->selectedRect().intersect(QRect(srcPos, areaSize));
            dstPos += r.topLeft() - srcPos;
            srcPos = r.topLeft();
            areaSize = r.size();
        }

        if (areaSize.width() == 0 || areaSize.height() == 0)
            return;

        addToProgress(0);
        if (isInterrupted()) return;

        const quint32 halfKernelWidth = (kernel->width() - 1) / 2;
        const quint32 halfKernelHeight = (kernel->height() - 1) / 2;

        m_fftWidth = areaSize.width() + 4 * halfKernelWidth;
        m_fftHeight = areaSize.height() + 2 * halfKernelHeight;

        /**
         * FIXME: check whether this "optimization" is needed to
         * be uncommented. My tests showed about 30% better performance
         * when the line is commented out (DK).
         */
        //optimumDimensions(m_fftWidth, m_fftHeight);

        m_fftLength = m_fftHeight * (m_fftWidth / 2 + 1);
        m_extraMem = (m_fftWidth % 2) ? 1 : 2;

        // create and fill kernel
        m_kernelFFT = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * m_fftLength);
        memset(m_kernelFFT, 0, sizeof(fftw_complex) * m_fftLength);
        fftFillKernelMatrix(kernel, m_kernelFFT);

        // find out which channels need convolving
        QList<KoChannelInfo*> convChannelList = this->convolvableChannelList(src);

        m_channelFFT.resize(convChannelList.count());
        for (auto i = m_channelFFT.begin(); i != m_channelFFT.end(); ++i) {
            *i = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * m_fftLength);
        }

        const double kernelFactor = kernel->factor() ? kernel->factor() : 1;
        const double fftScale = 1.0 / (m_fftHeight * m_fftWidth) / kernelFactor;

        FFTInfo info (fftScale, convChannelList, kernel, this->m_painter->device()->colorSpace());
        int cacheRowStride = m_fftWidth + m_extraMem;

        fillCacheFromDevice(src,
                            QRect(srcPos.x() - halfKernelWidth,
                                  srcPos.y() - halfKernelHeight,
                                  m_fftWidth,
                                  m_fftHeight),
                            cacheRowStride,
                            info, dataRect);

        addToProgress(10);
        if (isInterrupted()) return;

        // calculate number off fft operations required for progress reporting
        const float progressPerFFT = (100 - 30) / (double)(convChannelList.count() * 2 + 1);

        // perform FFT
        fftw_plan fftwPlanForward, fftwPlanBackward;

        KisConvolutionWorkerFFTLock::fftwMutex.lock();
        fftwPlanForward = fftw_plan_dft_r2c_2d(m_fftHeight, m_fftWidth, (double*)m_kernelFFT, m_kernelFFT, FFTW_ESTIMATE);
        fftwPlanBackward = fftw_plan_dft_c2r_2d(m_fftHeight, m_fftWidth, m_kernelFFT, (double*)m_kernelFFT, FFTW_ESTIMATE);
        KisConvolutionWorkerFFTLock::fftwMutex.unlock();

        fftw_execute(fftwPlanForward);
        addToProgress(progressPerFFT);
        if (isInterrupted()) return;

        for (auto k = m_channelFFT.begin(); k != m_channelFFT.end(); ++k)
        {
            fftw_execute_dft_r2c(fftwPlanForward, (double*)(*k), *k);
            addToProgress(progressPerFFT);
            if (isInterrupted()) return;

            fftMultiply(*k, m_kernelFFT);

            fftw_execute_dft_c2r(fftwPlanBackward, *k, (double*)*k);
            addToProgress(progressPerFFT);
            if (isInterrupted()) return;
        }

        KisConvolutionWorkerFFTLock::fftwMutex.lock();
        fftw_destroy_plan(fftwPlanForward);
        fftw_destroy_plan(fftwPlanBackward);
        KisConvolutionWorkerFFTLock::fftwMutex.unlock();


        writeResultToDevice(QRect(dstPos.x(), dstPos.y(), areaSize.width(), areaSize.height()),
                            cacheRowStride, halfKernelWidth, halfKernelHeight,
                            info, dataRect);

        addToProgress(20);
        cleanUp();
    }

    struct FFTInfo {
        FFTInfo(qreal _fftScale,
                QList<KoChannelInfo*> _convChannelList,
                const KisConvolutionKernelSP kernel,
                const KoColorSpace */*colorSpace*/)
            : fftScale(_fftScale),
              convChannelList(_convChannelList),
              alphaCachePos(-1),
              alphaRealPos(-1)
        {
            KisMathToolbox mathToolbox;

            for (int i = 0; i < convChannelList.count(); ++i) {
                minClamp.append(mathToolbox.minChannelValue(convChannelList[i]));
                maxClamp.append(mathToolbox.maxChannelValue(convChannelList[i]));
                absoluteOffset.append((maxClamp[i] - minClamp[i]) * kernel->offset());

                if (convChannelList[i]->channelType() == KoChannelInfo::ALPHA) {
                    alphaCachePos = i;
                    alphaRealPos = convChannelList[i]->pos();
                }
            }

            toDoubleFuncPtr.resize(convChannelList.count());
            fromDoubleFuncPtr.resize(convChannelList.count());

            bool result = mathToolbox.getToDoubleChannelPtr(convChannelList, toDoubleFuncPtr);
            result &= mathToolbox.getFromDoubleChannelPtr(convChannelList, fromDoubleFuncPtr);

            KIS_ASSERT(result);
        }

        inline int numChannels() const {
            return convChannelList.size();
        }


        QVector<qreal> minClamp;
        QVector<qreal> maxClamp;
        QVector<qreal> absoluteOffset;

        qreal fftScale;
        QList<KoChannelInfo*> convChannelList;

        QVector<PtrToDouble> toDoubleFuncPtr;
        QVector<PtrFromDouble> fromDoubleFuncPtr;

        int alphaCachePos;
        int alphaRealPos;
    };

    void fillCacheFromDevice(KisPaintDeviceSP src,
                             const QRect &rect,
                             const int cacheRowStride,
                             const FFTInfo &info,
                             const QRect &dataRect) {

        typename _IteratorFactory_::HLineConstIterator hitSrc =
            _IteratorFactory_::createHLineConstIterator(src,
                                                        rect.x(), rect.y(), rect.width(),
                                                        dataRect);

        QVector<double*> channelPtr(info.numChannels());

        for (int k = 0; k < channelPtr.size(); ++k) {
            channelPtr[k] = (double*)m_channelFFT[k];
        }

        for (int y = 0; y < rect.height(); ++y) {
            QVector<double*> cacheRowStart(channelPtr);

            for (int x = 0; x < rect.width(); ++x) {
                const quint8 *data = hitSrc->oldRawData();

                // no alpha is a rare case, so just multiply by 1.0 in that case
                double alphaValue = info.alphaRealPos >= 0 ?
                    info.toDoubleFuncPtr[info.alphaCachePos](data, info.alphaRealPos) : 1.0;

                for (int k = 0; k < channelPtr.size(); ++k) {
                    if (k != info.alphaCachePos) {
                        const quint32 channelPos = info.convChannelList[k]->pos();
                        *channelPtr[k] = info.toDoubleFuncPtr[k](data, channelPos) * alphaValue;
                    } else {
                        *channelPtr[k] = alphaValue;
                    }

                    channelPtr[k]++;
                }

                hitSrc->nextPixel();
            }

            for (int k = 0; k < channelPtr.size(); ++k) {
                channelPtr[k] = cacheRowStart[k] + cacheRowStride;
            }

            hitSrc->nextRow();
        }

    }


    inline void limitValue(qreal *value, qreal lowBound, qreal highBound) {
        if (*value > highBound) {
            *value = highBound;
        } else if (*value < lowBound){
            *value = lowBound;
        }
    }

    template <bool additionalMultiplierActive>
    inline qreal writeOneChannelFromCache(quint8* dstPtr,
                                          const quint32 channel,
                                          const int channelPos,
                                          const FFTInfo &info,
                                          const QVector<double*> &channelPtr,
                                          const qreal additionalMultiplier = 0.0) {
        qreal channelPixelValue;

        if (additionalMultiplierActive) {
            channelPixelValue = (*(channelPtr[channel]) * info.fftScale + info.absoluteOffset[channel]) * additionalMultiplier;
        } else {
            channelPixelValue = *(channelPtr[channel]) * info.fftScale + info.absoluteOffset[channel];
        }

        limitValue(&channelPixelValue, info.minClamp[channel], info.maxClamp[channel]);

        info.fromDoubleFuncPtr[channel](dstPtr, channelPos, channelPixelValue);

        return channelPixelValue;
    }

    void writeResultToDevice(const QRect &rect,
                             const int cacheRowStride,
                             const int halfKernelWidth,
                             const int halfKernelHeight,
                             const FFTInfo &info,
                             const QRect &dataRect) {

        typename _IteratorFactory_::HLineIterator hitDst =
            _IteratorFactory_::createHLineIterator(this->m_painter->device(),
                                                   rect.x(), rect.y(), rect.width(),
                                                   dataRect);

        int initialOffset = cacheRowStride * halfKernelHeight + halfKernelWidth;

        QVector<double*> channelPtr(info.numChannels());

        for (int k = 0; k < channelPtr.size(); ++k) {
            channelPtr[k] = (double*)m_channelFFT[k] + initialOffset;
        }

        for (int y = 0; y < rect.height(); ++y) {
            QVector<double*> cacheRowStart(channelPtr);

            for (int x = 0; x < rect.width(); ++x) {
                quint8 *dstPtr = hitDst->rawData();

                if (info.alphaCachePos >= 0) {
                    qreal alphaValue =
                        writeOneChannelFromCache<false>(dstPtr,
                                                        info.alphaCachePos,
                                                        info.convChannelList[info.alphaCachePos]->pos(),
                                                        info,
                                                        channelPtr);
                    qreal alphaValueInv = 1.0 / alphaValue;

                    for (int k = 0; k < channelPtr.size(); ++k) {
                        if (k != info.alphaCachePos) {
                            writeOneChannelFromCache<true>(dstPtr,
                                                           k,
                                                           info.convChannelList[k]->pos(),
                                                           info,
                                                           channelPtr,
                                                           alphaValueInv);
                        }
                        ++channelPtr[k];
                    }
                } else {
                    for (int k = 0; k < channelPtr.size(); ++k) {
                        writeOneChannelFromCache<false>(dstPtr,
                                                        k,
                                                        info.convChannelList[k]->pos(),
                                                        info,
                                                        channelPtr);
                        ++channelPtr[k];
                    }
                }

                hitDst->nextPixel();
            }

            for (int k = 0; k < channelPtr.size(); ++k) {
                channelPtr[k] = cacheRowStart[k] + cacheRowStride;
            }

            hitDst->nextRow();
        }

    }

private:
    void fftFillKernelMatrix(const KisConvolutionKernelSP kernel, fftw_complex *m_kernelFFT)
    {
        // find central item
        QPoint offset((kernel->width() - 1) / 2, (kernel->height() - 1) / 2);

        qint32 xShift = m_fftWidth - offset.x();
        qint32 yShift = m_fftHeight - offset.y();

        quint32 absXpos, absYpos;

        for (quint32 y = 0; y < kernel->height(); y++)
        {
            absYpos = y + yShift;
            if (absYpos >= m_fftHeight)
                absYpos -= m_fftHeight;

            for (quint32 x = 0; x < kernel->width(); x++)
            {
                absXpos = x + xShift;
                if (absXpos >= m_fftWidth)
                    absXpos -= m_fftWidth;

                ((double*)m_kernelFFT)[(m_fftWidth + m_extraMem) * absYpos + absXpos] = kernel->data()->coeff(y, x);
            }
        }
    }

    void fftMultiply(fftw_complex* channel, fftw_complex* kernel)
    {
        // perform complex multiplication
        fftw_complex *channelPtr = channel;
        fftw_complex *kernelPtr = kernel;

        fftw_complex tmp;

        for (quint32 pixelPos = 0; pixelPos < m_fftLength; ++pixelPos)
        {
            tmp[0] = ((*channelPtr)[0] * (*kernelPtr)[0]) - ((*channelPtr)[1] * (*kernelPtr)[1]);
            tmp[1] = ((*channelPtr)[0] * (*kernelPtr)[1]) + ((*channelPtr)[1] * (*kernelPtr)[0]);

            (*channelPtr)[0] = tmp[0];
            (*channelPtr)[1] = tmp[1];

            ++channelPtr;
            ++kernelPtr;
        }
    }

    void optimumDimensions(quint32& w, quint32& h)
    {
        // FFTW is most efficient when array size is a factor of 2, 3, 5 or 7
        quint32 optW = w, optH = h;

        while ((optW % 2 != 0) || (optW % 3 != 0) || (optW % 5 != 0) || (optW % 7 != 0))
            ++optW;

        while ((optH % 2 != 0) || (optH % 3 != 0) || (optH % 5 != 0) || (optH % 7 != 0))
            ++optH;

        quint32 optAreaW = optW * h;
        quint32 optAreaH = optH * w;

        if (optAreaW < optAreaH) {
            w = optW;
        }
        else {
            h = optH;
        }
    }

    void fftLogMatrix(double* channel, QString f)
    {
        KisConvolutionWorkerFFTLock::fftwMutex.lock();
        QString filename(QDir::homePath() + "/log_" + f + ".txt");
        dbgKrita << "Log File Name: " << filename;
        QFile file (filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            dbgKrita << "Failed";
            KisConvolutionWorkerFFTLock::fftwMutex.unlock();
            return;
        }

        QTextStream in(&file);
        for (quint32 y = 0; y < m_fftHeight; y++)
        {
            for (quint32 x = 0; x < m_fftWidth; x++)
            {
                QString num = QString::number(channel[y * m_fftWidth + x]);
                while (num.length() < 15)
                    num += " ";

                in << num << " ";
            }
            in << "\n";
        }
        KisConvolutionWorkerFFTLock::fftwMutex.unlock();
    }

    void addToProgress(float amount)
    {
        m_currentProgress += amount;

        if (this->m_progress) {
            this->m_progress->setProgress((int)m_currentProgress);
        }
    }

    bool isInterrupted()
    {
        if (this->m_progress && this->m_progress->interrupted()) {
            cleanUp();
            return true;
        }

        return false;
    }

    void cleanUp()
    {
        // free kernel fft data
        if (m_kernelFFT) {
            fftw_free(m_kernelFFT);
        }

        Q_FOREACH (fftw_complex *channel, m_channelFFT) {
            fftw_free(channel);
        }
        m_channelFFT.clear();
    }
private:
    quint32 m_fftWidth, m_fftHeight, m_fftLength, m_extraMem;
    float m_currentProgress;

    fftw_complex* m_kernelFFT;
    QVector<fftw_complex*> m_channelFFT;
};

#endif
