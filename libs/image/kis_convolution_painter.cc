/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#include "kis_convolution_painter.h"

#include <stdlib.h>
#include <string.h>
#include <cfloat>

#include <QBrush>
#include <QColor>
#include <QPen>
#include <QMatrix>
#include <QImage>
#include <QMap>
#include <QPainter>
#include <QRect>
#include <QString>
#include <QVector>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include "kis_convolution_kernel.h"
#include "kis_global.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "KoColorSpace.h"
#include <KoChannelInfo.h>
#include "kis_types.h"

#include "kis_selection.h"

#include "kis_convolution_worker.h"
#include "kis_convolution_worker_spatial.h"

#include "config_convolution.h"

#ifdef HAVE_FFTW3
#include "kis_convolution_worker_fft.h"
#endif


bool KisConvolutionPainter::useFFTImplemenation(const KisConvolutionKernelSP kernel) const
{
    bool result = false;

#ifdef HAVE_FFTW3
    #define THRESHOLD_SIZE 5

    result =
        m_enginePreference == FFTW ||
        (m_enginePreference == NONE &&
         (kernel->width() > THRESHOLD_SIZE ||
          kernel->height() > THRESHOLD_SIZE));
#else
    Q_UNUSED(kernel);
#endif

    return result;
}

template<class factory>
KisConvolutionWorker<factory>* KisConvolutionPainter::createWorker(const KisConvolutionKernelSP kernel,
                                                                   KisPainter *painter,
                                                                   KoUpdater *progress)
{
    KisConvolutionWorker<factory> *worker;

#ifdef HAVE_FFTW3
    if (useFFTImplemenation(kernel)) {
        worker = new KisConvolutionWorkerFFT<factory>(painter, progress);
    } else {
        worker = new KisConvolutionWorkerSpatial<factory>(painter, progress);
    }
#else
    Q_UNUSED(kernel);
    worker = new KisConvolutionWorkerSpatial<factory>(painter, progress);
#endif

    return worker;
}


KisConvolutionPainter::KisConvolutionPainter()
    : KisPainter(),
      m_enginePreference(NONE)
{
}

KisConvolutionPainter::KisConvolutionPainter(KisPaintDeviceSP device)
    : KisPainter(device),
      m_enginePreference(NONE)
{
}

KisConvolutionPainter::KisConvolutionPainter(KisPaintDeviceSP device, KisSelectionSP selection)
    : KisPainter(device, selection),
      m_enginePreference(NONE)
{
}

KisConvolutionPainter::KisConvolutionPainter(KisPaintDeviceSP device, TestingEnginePreference enginePreference)
    : KisPainter(device),
      m_enginePreference(enginePreference)
{
}

void KisConvolutionPainter::applyMatrix(const KisConvolutionKernelSP kernel, const KisPaintDeviceSP src, QPoint srcPos, QPoint dstPos, QSize areaSize, KisConvolutionBorderOp borderOp)
{
    /**
     * Force BORDER_IGNORE op for the wraparound mode,
     * because the paint device has its own special
     * iterators, which do everything for us.
     */
    if (src->defaultBounds()->wrapAroundMode()) {
        borderOp = BORDER_IGNORE;
    }

    // Determine whether we convolve border pixels, or not.
    switch (borderOp) {
    case BORDER_REPEAT: {
        const QRect boundsRect = src->exactBounds();
        const QRect requestedRect = QRect(srcPos, areaSize);
        QRect dataRect = requestedRect | boundsRect;

        /**
         * FIXME: Implementation can return empty destination device
         * on faults and has no way to report this. This will cause a crash
         * on sequential convolutions inside iteratiors.
         *
         * o implementation should do it's work or assert otherwise
         *   (or report the issue somehow)
         * o check other cases of the switch for the vulnerability
         */

        if(dataRect.isValid()) {
            KisConvolutionWorker<RepeatIteratorFactory> *worker;
            worker = createWorker<RepeatIteratorFactory>(kernel, this, progressUpdater());
            worker->execute(kernel, src, srcPos, dstPos, areaSize, dataRect);
            delete worker;
        }
        break;
    }
    case BORDER_IGNORE:
    default: {
        KisConvolutionWorker<StandardIteratorFactory> *worker;
        worker = createWorker<StandardIteratorFactory>(kernel, this, progressUpdater());
        worker->execute(kernel, src, srcPos, dstPos, areaSize, QRect());
        delete worker;
    }
    }
}

bool KisConvolutionPainter::needsTransaction(const KisConvolutionKernelSP kernel) const
{
    return !useFFTImplemenation(kernel);
}
