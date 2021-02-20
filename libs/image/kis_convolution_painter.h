/*
 *  SPDX-FileCopyrightText: 2005 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CONVOLUTION_PAINTER_H_
#define KIS_CONVOLUTION_PAINTER_H_

#include "kis_types.h"
#include "kis_painter.h"
#include "kis_image.h"
#include "kritaimage_export.h"

template<class factory> class KisConvolutionWorker;


enum KisConvolutionBorderOp {
    BORDER_IGNORE = 0, // read the pixels outside of the application rect
    BORDER_REPEAT = 1  // Use the border for the missing pixels
};

/**
 * @brief The KisConvolutionPainter class applies a convolution kernel to a paint device.
 *
 *
 * Note: https://bugs.kde.org/show_bug.cgi?id=220310 shows that there's something here
 * that we need to fix...
 */
class KRITAIMAGE_EXPORT KisConvolutionPainter : public KisPainter
{

public:

    KisConvolutionPainter();
    KisConvolutionPainter(KisPaintDeviceSP device);
    KisConvolutionPainter(KisPaintDeviceSP device, KisSelectionSP selection);

    enum EnginePreference {
        NONE,
        SPATIAL,
        FFTW
    };


    KisConvolutionPainter(KisPaintDeviceSP device, EnginePreference enginePreference);

    void setEnginePreference(EnginePreference value);

    /**
     * Convolve all channels in src using the specified kernel; there is only one kernel for all
     * channels possible.
     *
     * WARNING: The painter will read **more** pixels than you pass in \p areaSize.
     *          The actual processing area will be:
     *          QRect(x - kernel.width() / 2,
     *                y - kernel.height() / 2,
     *                w + 2 * (kernel.width() / 2),
     *                h + 2 * (kernel.height() / 2))
     *
     * The border op decides what to do with pixels too close to the edge of the rect as defined above.
     *
     * The channels flag determines which set out of color channels, alpha channels.
     * channels we convolve.
     *
     * Note that we do not (currently) support different kernels for
     * different channels _or_ channel types.
     *
     * If you want to convolve a subset of the channels in a pixel,
     * set those channels with KisPainter::setChannelFlags();
     */
    void applyMatrix(const KisConvolutionKernelSP kernel, const KisPaintDeviceSP src, QPoint srcPos, QPoint dstPos, QSize areaSize,
                     KisConvolutionBorderOp borderOp = BORDER_REPEAT);

    /**
     * The caller should ask if the painter needs an explicit transaction iff
     * the source and destination devices coincide. Otherwise, the transaction is
     * just not needed.
     */
    bool needsTransaction(const KisConvolutionKernelSP kernel) const;

    static bool supportsFFTW();

protected:
    friend class KisConvolutionPainterTest;



private:
    template<class factory>
        KisConvolutionWorker<factory>* createWorker(const KisConvolutionKernelSP kernel,
                                                    KisPainter *painter,
                                                    KoUpdater *progress);

     bool useFFTImplementation(const KisConvolutionKernelSP kernel) const;

private:
    EnginePreference m_enginePreference;
};
#endif //KIS_CONVOLUTION_PAINTER_H_
