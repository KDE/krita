/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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


bool KisConvolutionPainter::useFFTImplementation(const KisConvolutionKernelSP kernel) const
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
    if (useFFTImplementation(kernel)) {
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


bool KisConvolutionPainter::supportsFFTW()
{
#ifdef HAVE_FFTW3
    return true;
#else
    return false;
#endif
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

KisConvolutionPainter::KisConvolutionPainter(KisPaintDeviceSP device, EnginePreference enginePreference)
    : KisPainter(device),
      m_enginePreference(enginePreference)
{
}

void KisConvolutionPainter::setEnginePreference(EnginePreference value)
{
    m_enginePreference = value;
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
        /**
         * We don't use defaultBounds->topLevelWrapRect(), because
         * the main purpose of this wrapping is "getting expected
         * results when applying to the the layer". If a mask is bigger
         * than the image, then it should be wrapped around the mask
         * instead.
         */
        const QRect boundsRect = src->defaultBounds()->bounds();
        const QRect requestedRect = QRect(srcPos, areaSize);
        QRect dataRect = requestedRect | boundsRect;

        KIS_SAFE_ASSERT_RECOVER(boundsRect != KisDefaultBounds().bounds()) {
            dataRect = requestedRect | src->exactBounds();
        }

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
    return !useFFTImplementation(kernel);
}
