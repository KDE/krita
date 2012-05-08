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

#include "fftw3.h"

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
    KisConvolutionWorkerFFT(KisPainter *painter, KoUpdater *progress) : KisConvolutionWorker<_IteratorFactory_>(painter, progress),
            m_currentProgress(0)
    {
        m_kernelFFT = 0;
        m_channelFFT = 0;

        m_minClamp = 0;
        m_maxClamp = 0;
        m_absoluteOffset = 0;

        m_channelPtr = 0;
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

        m_fftWidth = areaSize.width() + kernel->width() - 1;
        m_fftHeight = areaSize.height() + kernel->height() - 1;
        optimumDimensions(m_fftWidth, m_fftHeight);

        m_fftLength = m_fftHeight * (m_fftWidth / 2 + 1);
        m_extraMem = m_fftWidth % 2 ? 1 : 2;

        // create and fill kernel
        m_kernelFFT = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * m_fftLength);
        memset(m_kernelFFT, 0, sizeof(fftw_complex) * m_fftLength);
        fftFillKernelMatrix(kernel, m_kernelFFT);

        // find out which channels need convolving
        QList<KoChannelInfo *> convChannelList = this->convolvableChannelList(src);
        m_noOfChannels = convChannelList.count();

        // Pentalis comment: Find out if one of those is the alpha channel
        qint32 alphaChannelIndex = -1;  //-1 = FALSE
        for (quint32 i = 0; i < m_noOfChannels; ++i) {
            if (convChannelList.at(i)->channelType() == KoChannelInfo::ALPHA) {
                alphaChannelIndex = i;
            }
        }
        
        m_channelFFT = new fftw_complex*[m_noOfChannels];
        for (quint32 i = 0; i < m_noOfChannels; ++i)
            m_channelFFT[i] = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * m_fftLength);

        // fill in function
        QVector<PtrToDouble> toDoubleFuncPtr(m_noOfChannels);

        KisMathToolbox* mathToolbox = KisMathToolboxRegistry::instance()->value(src->colorSpace()->mathToolboxId().id());
        if (!mathToolbox->getToDoubleChannelPtr(convChannelList, toDoubleFuncPtr))
            return;

        m_channelPtr = new double*[m_noOfChannels];
        for (quint32 k = 0; k < m_noOfChannels; ++k)
            m_channelPtr[k] = (double*)m_channelFFT[k];

        typename _IteratorFactory_::HLineConstIterator hitSrc = _IteratorFactory_::createHLineConstIterator(src, srcPos.x() - halfKernelWidth, srcPos.y() - halfKernelHeight, m_fftWidth, dataRect);
        
        for (quint32 srcRow = 0; srcRow < m_fftHeight; ++srcRow) {

            do {
                for (quint32 k = 0; k < m_noOfChannels; ++k) {
                    *m_channelPtr[k]++ = toDoubleFuncPtr[k](hitSrc->oldRawData(), convChannelList[k]->pos());
                }
            } while (hitSrc->nextPixel());

            for (quint32 k = 0; k < m_noOfChannels; ++k) {
                m_channelPtr[k] += m_extraMem;
            }

            hitSrc->nextRow();
        }
        
        // Pentalis comment: All the code below is devoted to premultiplying alpha, obviously there's no need for that if there's no alpha channel in the convolution list
        if (alphaChannelIndex >= 0)
        {
            // Pentalis comment: Resetting m_channelPtr to the start (according to the loop above), or what I think is the start
            for (quint32 k = 0; k < m_noOfChannels; ++k)
                m_channelPtr[k] = (double*)m_channelFFT[k];
            
            // Pentalis comment: I'll keep trying till this thing works and I never have to touch it again  UPDATE: WORKED!
            
            // Pentalis comment: Iterate over the same pixels again but this time with a different intent
            for (quint32 srcRow = 0; srcRow < m_fftHeight; ++srcRow)
            {
                for (quint32 srcCol = 0; srcCol < m_fftWidth; ++srcCol)
                {
                    for (quint32 k = 0; k < m_noOfChannels; ++k) {
                        // Pentalis comment: Pass to the next loop if you hit the alpha channel,
                        // make sure to increment *m_channelPtr[k]  (this was a bug that took hours to find)
                        if (k == (quint32)alphaChannelIndex) {
                            *m_channelPtr[k]++;   // careful, this increment is deep Hocus Pocus, don't touch unless you know what you're doing
                            continue;
                        }
                        
                        // Pentalis comments: PREMULTIPLY BY ALPHA
                        // This code works because m_channelPtr has already been filled entirely
                        *m_channelPtr[k] *= *m_channelPtr[alphaChannelIndex];
                        *m_channelPtr[k]++;   // careful, this increment is deep Hocus Pocus, don't touch unless you know what you're doing
                    }
                }

                for (quint32 k = 0; k < m_noOfChannels; ++k) {
                    m_channelPtr[k] += m_extraMem;
                }
            }
        }
        
        addToProgress(10);
        if (isInterrupted()) return;
        
        // calculate number off fft operations required for progress reporting
        const float progressPerFFT = (100 - 30) / (double)(m_noOfChannels * 2 + 1);

        // perform FFT
        fftw_plan fftwPlanForward, fftwPlanBackward;

        KisConvolutionWorkerFFTLock::fftwMutex.lock();
        fftwPlanForward = fftw_plan_dft_r2c_2d(m_fftHeight, m_fftWidth, (double*)m_kernelFFT, m_kernelFFT, FFTW_ESTIMATE);
        fftwPlanBackward = fftw_plan_dft_c2r_2d(m_fftHeight, m_fftWidth, m_kernelFFT, (double*)m_kernelFFT, FFTW_ESTIMATE);
        KisConvolutionWorkerFFTLock::fftwMutex.unlock();

        fftw_execute(fftwPlanForward);
        addToProgress(progressPerFFT);
        if (isInterrupted()) return;

        for (quint32 k = 0; k < m_noOfChannels; ++k)
        {
            fftw_execute_dft_r2c(fftwPlanForward, (double*)(m_channelFFT[k]), m_channelFFT[k]);
            addToProgress(progressPerFFT);
            if (isInterrupted()) return;

            fftMultiply(m_channelFFT[k], m_kernelFFT);

            fftw_execute_dft_c2r(fftwPlanBackward, m_channelFFT[k], (double*)m_channelFFT[k]);
            addToProgress(progressPerFFT);
            if (isInterrupted()) return;
        }

        KisConvolutionWorkerFFTLock::fftwMutex.lock();
        fftw_destroy_plan(fftwPlanForward);
        fftw_destroy_plan(fftwPlanBackward);
        KisConvolutionWorkerFFTLock::fftwMutex.unlock();

        // calculate final result
        const double kernelFactor = kernel->factor() ? kernel->factor() : 1;
        const double fftScale = 1.0 / (m_fftHeight * m_fftWidth) / kernelFactor;
        const quint32 areaWidth = areaSize.width();
        const quint32 areaHeight = areaSize.height();

        // offset pointers
        const quint32 rowOffsetPtr = m_fftWidth - areaSize.width() + m_extraMem;
        const quint32 offsetPtr = (2 * (m_fftWidth/2 + 1)) * halfKernelHeight + halfKernelWidth;
        for (quint32 k = 0; k < m_noOfChannels; ++k)
            m_channelPtr[k] = (double*)m_channelFFT[k] + offsetPtr;

        QVector<PtrFromDouble> fromDoubleFuncPtr(m_noOfChannels);
        if (!mathToolbox->getFromDoubleChannelPtr(convChannelList, fromDoubleFuncPtr))
            return;

        typename _IteratorFactory_::HLineIterator hitDst = _IteratorFactory_::createHLineIterator(this->m_painter->device(), dstPos.x(), dstPos.y(), areaSize.width(), dataRect);
        typename _IteratorFactory_::HLineIterator hitSrcCpy = _IteratorFactory_::createHLineIterator(src, srcPos.x(), srcPos.y(), areaSize.width(), dataRect);

        double channelPixelValue;

        double *m_maxClamp = new double[convChannelList.count()];
        double *m_minClamp = new double[convChannelList.count()];
        m_absoluteOffset = new double[convChannelList.count()];
        for (quint16 i = 0; i < convChannelList.count(); ++i)
        {
            m_minClamp[i] = mathToolbox->minChannelValue(convChannelList[i]);
            m_maxClamp[i] = mathToolbox->maxChannelValue(convChannelList[i]);
            m_absoluteOffset[i] = (m_maxClamp[i] - m_minClamp[i]) * kernel->offset();
        }

        // Pentalis comment: if there IS an alpha Channel...
        if (alphaChannelIndex >= 0)
        {
            double alphaChannelPixelValue;
            for (quint32 y = 0; y < areaHeight; ++y)
            {
                for (quint32 x = 0; x < areaWidth; ++x)
                {
                    // Pentalis comment: This needs to be done only once and before we iterate over the channels
                    alphaChannelPixelValue = *(m_channelPtr[alphaChannelIndex]) * fftScale + m_absoluteOffset[alphaChannelIndex];
                    
                    for (quint32 k = 0; k < m_noOfChannels; ++k)
                    {
                        channelPixelValue = *(m_channelPtr[k]) * fftScale + m_absoluteOffset[k];
                        
                        if (k != (quint32)alphaChannelIndex) {
                            // Pentalis comment: divide the PREMULTIPLIED (see conditionals above) channels by the
                            // CONVOLUTED alpha channel. Also, avoid division by zero.
                            if (alphaChannelIndex != 0) {
                                channelPixelValue /= alphaChannelPixelValue;
                            }
                        }
                        
                        // clamp values
                        if (channelPixelValue > m_maxClamp[k])
                            channelPixelValue = m_maxClamp[k];
                        else if (channelPixelValue < m_minClamp[k])
                            channelPixelValue = m_minClamp[k];

                        fromDoubleFuncPtr[k](hitDst->rawData(), convChannelList[k]->pos(), channelPixelValue);

                        ++m_channelPtr[k];
                    }

                    hitDst->nextPixel();
                    hitSrcCpy->nextPixel();
                }

                for (quint32 k = 0; k < m_noOfChannels; ++k)
                    m_channelPtr[k] += rowOffsetPtr;

                hitDst->nextRow();
                hitSrcCpy->nextRow();
            }
        }
        else 
        {
            
            for (quint32 y = 0; y < areaHeight; ++y)
            {
                for (quint32 x = 0; x < areaWidth; ++x)
                {
                    for (quint32 k = 0; k < m_noOfChannels; ++k)
                    {
                        channelPixelValue = *(m_channelPtr[k]) * fftScale + m_absoluteOffset[k];
                        
                        // clamp values
                        if (channelPixelValue > m_maxClamp[k])
                            channelPixelValue = m_maxClamp[k];
                        else if (channelPixelValue < m_minClamp[k])
                            channelPixelValue = m_minClamp[k];

                        fromDoubleFuncPtr[k](hitDst->rawData(), convChannelList[k]->pos(), channelPixelValue);

                        ++m_channelPtr[k];
                    }

                    hitDst->nextPixel();
                    hitSrcCpy->nextPixel();
                }

                for (quint32 k = 0; k < m_noOfChannels; ++k)
                    m_channelPtr[k] += rowOffsetPtr;

                hitDst->nextRow();
                hitSrcCpy->nextRow();
            }
        }
        
        addToProgress(20);
        cleanUp();
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
        fftw_complex *m_channelPtr = channel;
        fftw_complex *kernelPtr = kernel;

        fftw_complex tmp;

        for (quint32 pixelPos = 0; pixelPos < m_fftLength; ++pixelPos)
        {
            tmp[0] = ((*m_channelPtr)[0] * (*kernelPtr)[0]) - ((*m_channelPtr)[1] * (*kernelPtr)[1]);
            tmp[1] = ((*m_channelPtr)[0] * (*kernelPtr)[1]) + ((*m_channelPtr)[1] * (*kernelPtr)[0]);

            (*m_channelPtr)[0] = tmp[0];
            (*m_channelPtr)[1] = tmp[1];

            ++m_channelPtr;
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
        qDebug() << "Log File Name: " << filename;
        QFile file (filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            qDebug() << "Failed";
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

        // free channel fft data
        if (m_channelFFT) {
            for (uint i = 0; i < m_noOfChannels; ++i) {
                fftw_free(m_channelFFT[i]);
            }

            delete[] m_channelFFT;
        }

        if (m_minClamp) delete[] m_minClamp;
        if (m_maxClamp) delete[] m_maxClamp;
        if (m_absoluteOffset) delete[] m_absoluteOffset;
        if (m_channelPtr) delete[] m_channelPtr;
    }
private:
    quint32 m_fftWidth, m_fftHeight, m_fftLength, m_noOfChannels, m_extraMem;
    float m_currentProgress;

    fftw_complex* m_kernelFFT;
    fftw_complex** m_channelFFT;
    double* m_minClamp, *m_maxClamp, *m_absoluteOffset;
    double** m_channelPtr;
};

#endif
