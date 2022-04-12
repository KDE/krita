/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SCANLINE_FILL_H
#define __KIS_SCANLINE_FILL_H

#include <QScopedPointer>

#include <kritaimage_export.h>
#include <kis_types.h>
#include <kis_paint_device.h>

class KisFillInterval;
class KisFillIntervalMap;

class KRITAIMAGE_EXPORT KisScanlineFill
{
public:
    KisScanlineFill(KisPaintDeviceSP device, const QPoint &startPoint, const QRect &boundingRect);
    ~KisScanlineFill();

    /**
     * Fill the source device with \p fillColor
     */
    void fillColor(const KoColor &fillColor);

    /**
     * Fill \p externalDevice with \p fillColor basing on the contents
     * of the source device.
     */
    void fillColor(const KoColor &fillColor, KisPaintDeviceSP externalDevice);

    /**
     * Fill \p pixelSelection with the opacity of the contiguous area.
     * This method uses an existing selection as boundary for the flood fill.
     */
    void fillSelectionWithBoundary(KisPixelSelectionSP pixelSelection, KisPaintDeviceSP existingSelection);

    /**
     * Fill \p pixelSelection with the opacity of the contiguous area
     */
    void fillSelection(KisPixelSelectionSP pixelSelection);

    /**
     * Fill \p pixelSelection with the opacity of the contiguous area, which
     * encompass all the connected pixels as long as the color in the
     * pixels of the source device is not similar to \p referenceColor.
     * This method uses an existing selection as boundary for the flood fill.
     */
    void fillSelectionUntilColorWithBoundary(KisPixelSelectionSP pixelSelection, const KoColor &referenceColor, KisPaintDeviceSP existingSelection);

    /**
     * Fill \p pixelSelection with the opacity of the contiguous area, which
     * encompass all the connected pixels as long as the color in the
     * pixels of the source device is not similar to \p referenceColor.
     */
    void fillSelectionUntilColor(KisPixelSelectionSP pixelSelection, const KoColor &referenceColor);

    /**
     * Fill \p pixelSelection with the opacity of the contiguous area, which
     * encompass all the connected pixels as long as the color in the
     * pixels of the source device is not similar to \p referenceColor or transparent.
     * This method uses an existing selection as boundary for the flood fill.
     */
    void fillSelectionUntilColorOrTransparentWithBoundary(KisPixelSelectionSP pixelSelection, const KoColor &referenceColor, KisPaintDeviceSP existingSelection);

    /**
     * Fill \p pixelSelection with the opacity of the contiguous area, which
     * encompass all the connected pixels as long as the color in the
     * pixels of the source device is not similar to \p referenceColor or transparent.
     */
    void fillSelectionUntilColorOrTransparent(KisPixelSelectionSP pixelSelection, const KoColor &referenceColor);

    /**
     * Clear the contiguous non-zero area of the device
     *
     * WARNING: the threshold parameter is not counted!
     */
    void clearNonZeroComponent();

    /**
     * A special filler algorithm for the Watershed initialization routine:
     *
     * 1) Clear the contiguous area in the destination device
     * 2) At the same time, fill the corresponding area of \p groupMapDevice with
     *    value \p groupIndex
     * 3) \p groupMapDevice **must** store 4 bytes per pixel
     */
    void fillContiguousGroup(KisPaintDeviceSP groupMapDevice, qint32 groupIndex);

    /**
     * Set the threshold of the filling operation
     *
     * Used in all functions except clearNonZeroComponent()
     */
    void setThreshold(int threshold);

    /**
     * Set the opacity spread for floodfill. The range is 0-100: 0% means that
     * the fully opaque area only encompasses the pixels exactly equal to the
     * seed point with the other pixels of the selected region being
     * semi-transparent (depending on how similar they are to the seed pixel)
     * up to the region boundary (given by the threshold value). 100 means that
     * the fully opaque area will emcompass all the pixels of the selected
     * region up to the contour. Any value inbetween will make the fully opaque
     * portion of the region vary in size, with semi-transparent pixels
     * inbetween it and  the region boundary
     */
    void setOpacitySpread(int opacitySpread);

private:
    friend class KisScanlineFillTest;
    Q_DISABLE_COPY(KisScanlineFill)

    template <class T>
    void processLine(KisFillInterval interval, const int rowIncrement, T &pixelPolicy);


    template <class T>
        void extendedPass(KisFillInterval *currentInterval, int srcRow, bool extendRight, T &pixelPolicy);

    template <class T>
    void runImpl(T &pixelPolicy);

private:
    void testingProcessLine(const KisFillInterval &processInterval);
    QVector<KisFillInterval> testingGetForwardIntervals() const;
    KisFillIntervalMap* testingGetBackwardIntervals() const;
private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SCANLINE_FILL_H */
