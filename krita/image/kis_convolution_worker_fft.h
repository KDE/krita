/*
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

#ifndef KIS_CONVOLUTION_WORKER_FFT_H
#define KIS_CONVOLUTION_WORKER_FFT_H

#include <KoChannelInfo.h>

#include "kis_convolution_worker.h"
#include "kis_math_toolbox.h"

#include <QMutex>
#include <QVector>
#include <QTextStream>
#include <QFile>
#include <QDir>

#include "fftw3.h"

template <class _IteratorFactory_> class KisConvolutionWorkerFFT;
class KisConvolutionWorkerFFTLock
{
private:
    static QMutex fftwMutex;
    template <class _IteratorFactory_> friend class KisConvolutionWorkerFFT;
};

QMutex KisConvolutionWorkerFFTLock::fftwMutex;


template <class _IteratorFactory_>
class KisConvolutionWorkerFFT : public KisConvolutionWorker<_IteratorFactory_>
{
public:
    KisConvolutionWorkerFFT(KisPainter *painter, KoUpdater *progress) : KisConvolutionWorker<_IteratorFactory_>(painter, progress) {
    }

    ~KisConvolutionWorkerFFT() {
    }

    virtual void execute(const KisConvolutionKernelSP kernel, const KisPaintDeviceSP src, QPoint srcPos, QPoint dstPos, QSize areaSize, const QRect& dataRect)
    {
        // Make the area we cover as small as possible
        if (this->m_painter->selection()) {
            QRect r = this->m_painter->selection()->selectedRect().intersect(QRect(srcPos, areaSize));
            dstPos += r.topLeft() - srcPos;
            srcPos = r.topLeft();
            areaSize = r.size();
        }

        if (areaSize.width() == 0 || areaSize.height() == 0)
            return;

        const quint32 halfKernelWidth = (kernel->width() - 1) / 2;
        const quint32 halfKernelHeight = (kernel->height() - 1) / 2;

        m_fftWidth = areaSize.width() + kernel->width() - 1;
        m_fftHeight = areaSize.height() + kernel->height() - 1;

        // to simplify things, force width and height to be the same
        if (m_fftWidth > m_fftHeight) m_fftHeight = m_fftWidth;
        if (m_fftHeight > m_fftWidth) m_fftWidth = m_fftHeight;
        const quint32 fftLength = m_fftWidth * m_fftHeight;

        qDebug() << "------------------------";
        qDebug() << "Data Tile: " << areaSize;
        qDebug() << "FFT Width: " << m_fftWidth << " FFT Height: " << m_fftHeight;

        // create and fill kernel
        fftw_complex *kernelFFT = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * fftLength);
        memset(kernelFFT, 0, fftLength * sizeof(fftw_complex));
        fftFillKernelMatrix(kernel, kernelFFT);

        // find out which channels need be convolved
        QList<KoChannelInfo *> convChannelList = this->convolvableChannelList(src);
        const quint32 noOfChannels = convChannelList.count();

        fftw_complex** channelFFT = new fftw_complex*[noOfChannels];
        for (quint32 i = 0; i < noOfChannels; ++i) {
            channelFFT[i] = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * fftLength);
            memset(channelFFT[i], 0, fftLength * sizeof(fftw_complex));
        }

        // fill in data
        QVector<PtrToDouble> toDoubleFuncPtr(noOfChannels);

        KisMathToolbox* mathToolbox = KisMathToolboxRegistry::instance()->value(src->colorSpace()->mathToolboxId().id());
        if (!mathToolbox->getToDoubleChannelPtr(convChannelList, toDoubleFuncPtr))
            return;

        typename _IteratorFactory_::HLineConstIterator hitSrc = _IteratorFactory_::createHLineConstIterator(src, srcPos.x() - halfKernelWidth, srcPos.y() - halfKernelHeight, m_fftWidth, dataRect);
        quint32 pixelNo = 0;

        for (quint32 srcRow = 0; srcRow < m_fftHeight; ++srcRow) {

            while (!hitSrc.isDone()) {

                const quint8* data = hitSrc.oldRawData();

                for (quint32 k = 0; k < noOfChannels; ++k) {
                    channelFFT[k][pixelNo][0] = toDoubleFuncPtr[k](data, convChannelList[k]->pos());
                }

                ++pixelNo;
                ++hitSrc;
            }

            hitSrc.nextRow();
        }

        // perform FFT
        fftw_plan kernelPlan, channelPlan;

        KisConvolutionWorkerFFTLock::fftwMutex.lock();
        kernelPlan = fftw_plan_dft_2d(m_fftWidth, m_fftHeight, kernelFFT, kernelFFT, FFTW_FORWARD, FFTW_ESTIMATE);
        KisConvolutionWorkerFFTLock::fftwMutex.unlock();

        fftw_execute(kernelPlan);

        KisConvolutionWorkerFFTLock::fftwMutex.lock();
        fftw_destroy_plan(kernelPlan);
        KisConvolutionWorkerFFTLock::fftwMutex.unlock();

        for (quint32 k = 0; k < noOfChannels; ++k) {

            KisConvolutionWorkerFFTLock::fftwMutex.lock();
            channelPlan = fftw_plan_dft_2d(m_fftWidth, m_fftHeight, channelFFT[k], channelFFT[k], FFTW_FORWARD, FFTW_ESTIMATE);
            KisConvolutionWorkerFFTLock::fftwMutex.unlock();

            fftw_execute(channelPlan);

            KisConvolutionWorkerFFTLock::fftwMutex.lock();
            fftw_destroy_plan(channelPlan);
            KisConvolutionWorkerFFTLock::fftwMutex.unlock();
        }

        // multiply FFTs
        for (quint32 k = 0; k < noOfChannels; ++k) {
            fftMultiply(channelFFT[k], kernelFFT);
        }

        // calculate Inverse FFT
        for (quint32 k = 0; k < noOfChannels; ++k) {
            KisConvolutionWorkerFFTLock::fftwMutex.lock();
            channelPlan = fftw_plan_dft_2d(m_fftWidth, m_fftHeight, channelFFT[k], channelFFT[k], FFTW_BACKWARD, FFTW_ESTIMATE);
            KisConvolutionWorkerFFTLock::fftwMutex.unlock();

            fftw_execute(channelPlan);

            KisConvolutionWorkerFFTLock::fftwMutex.lock();
            fftw_destroy_plan(channelPlan);
            KisConvolutionWorkerFFTLock::fftwMutex.unlock();
        }

        // calculate final result
        const double kernelFactor = kernel->factor() ? kernel->factor() : 1;
        const double fftScale = 1.0 / fftLength / kernelFactor;
        const quint32 areaWidth = areaSize.width();
        const quint32 areaHeight = areaSize.height();

        fftw_complex** channelPtr = new fftw_complex*[noOfChannels];
        // offset pointers
        const quint32 rowOffsetPtr = m_fftWidth - areaSize.width();
        const quint32 offsetPtr = m_fftWidth * halfKernelHeight + halfKernelWidth;
        for (quint32 k = 0; k < noOfChannels; ++k)
            channelPtr[k] = channelFFT[k] + offsetPtr;

        QVector<PtrFromDouble> fromDoubleFuncPtr(noOfChannels);
        if (!mathToolbox->getFromDoubleChannelPtr(convChannelList, fromDoubleFuncPtr))
            return;

        typename _IteratorFactory_::HLineIterator hitDst = _IteratorFactory_::createHLineIterator(this->m_painter->device(), dstPos.x(), dstPos.y(), areaSize.width(), dataRect);
        typename _IteratorFactory_::HLineIterator hitSrcCpy = _IteratorFactory_::createHLineIterator(src, srcPos.x(), srcPos.y(), areaSize.width(), dataRect);

        double channelPixelValue;

        double *maxClamp = new double[convChannelList.count()];
        double *minClamp = new double[convChannelList.count()];
        double *absoluteOffset = new double[convChannelList.count()];
        for (quint16 i = 0; i < convChannelList.count(); ++i) {
            minClamp[i] = mathToolbox->minChannelValue(convChannelList[i]);
            maxClamp[i] = mathToolbox->maxChannelValue(convChannelList[i]);
            absoluteOffset[i] = (maxClamp[i] - minClamp[i]) * kernel->offset();
        }

        const quint32 pixelSize = src->colorSpace()->pixelSize();

        for (quint32 y = 0; y < areaHeight; ++y) {

            for (quint32 x = 0; x < areaWidth; ++x) {

                quint8 *data = hitDst.rawData();
                memcpy(hitDst.rawData(), hitSrcCpy.oldRawData(), pixelSize);

                for (quint32 k = 0; k < noOfChannels; ++k) {

                    channelPixelValue = (double)(*(channelPtr[k][0])) * fftScale + absoluteOffset[k];

                    // clamp values
                    if (channelPixelValue > maxClamp[k])
                        channelPixelValue = maxClamp[k];
                    else if (channelPixelValue < minClamp[k])
                        channelPixelValue = minClamp[k];

                    fromDoubleFuncPtr[k](data, convChannelList[k]->pos(), channelPixelValue);

                    ++channelPtr[k];
                }

                ++hitDst;
                ++hitSrcCpy;
            }

            for (quint32 k = 0; k < noOfChannels; ++k)
                channelPtr[k] += rowOffsetPtr;

            hitDst.nextRow();
            hitSrcCpy.nextRow();
        }

        // free memory
        fftw_free(kernelFFT);
        for (quint32 k = 0; k < noOfChannels; ++k) {
            fftw_free(channelFFT[k]);
        }

        delete[] channelFFT;
        delete[] channelPtr;
        delete[] minClamp;
        delete[] maxClamp;
        delete[] absoluteOffset;
    }

    void fftFillKernelMatrix(const KisConvolutionKernelSP kernel, fftw_complex *kernelFFT)
    {
        // find central item
        QPoint offset((kernel->width() - 1) / 2, (kernel->height() - 1) / 2);

        qint32 absXpos, absYpos;

        for (quint32 y = 0; y < kernel->height(); y++)
        {
            absYpos = y - offset.y();
            if (absYpos < 0)
                absYpos = m_fftHeight + absYpos;

            for (quint32 x = 0; x < kernel->width(); x++)
            {
                absXpos = x - offset.x();
                if (absXpos < 0)
                    absXpos = m_fftWidth + absXpos;

                kernelFFT[m_fftWidth * absYpos + absXpos][0] = (*(kernel->data()))(y, x);
            }
        }
    }

    void fftMultiply(fftw_complex* channel, fftw_complex* kernel)
    {
        // perform complex multiplication
        const quint32 fftLength = m_fftWidth * m_fftHeight;

        fftw_complex *channelPtr = channel;
        fftw_complex *kernelPtr = kernel;

        fftw_complex tmp;

        for (quint32 pixelPos = 0; pixelPos < fftLength; ++pixelPos)
        {
            tmp[0] = ((*channelPtr)[0] * (*kernelPtr)[0]) - ((*channelPtr)[1] * (*kernelPtr)[1]);
            tmp[1] = ((*channelPtr)[0] * (*kernelPtr)[1]) + ((*channelPtr)[1] * (*kernelPtr)[0]);

            (*channelPtr)[0] = tmp[0];
            (*channelPtr)[1] = tmp[1];

            ++channelPtr;
            ++kernelPtr;
        }
    }
private:
    void fftLogMatrix(fftw_complex* channel, QString f)
    {
        KisConvolutionWorkerFFTLock::fftwMutex.lock();
        QString filename(QDir::homePath() + "log_" + f);
        qDebug() << "Log File Name: " << filename;
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            qDebug() << "Failed";
            KisConvolutionWorkerFFTLock::fftwMutex.unlock();
            return;
        }

        QTextStream in(&file);
        for (quint32 y = 0; y < m_fftHeight; y++) {
            for (quint32 x = 0; x < m_fftWidth; x++) {
                QString num = QString::number(channel[y * m_fftWidth + x][0]);
                while (num.length() < 15)
                    num += " ";

                in << num << " ";
            }
            in << "\n";
        }
        KisConvolutionWorkerFFTLock::fftwMutex.unlock();
    }

private:
    quint32 m_fftWidth, m_fftHeight;
};

#endif
