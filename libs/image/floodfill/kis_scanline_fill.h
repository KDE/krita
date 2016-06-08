/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

    void fillColor(const KoColor &fillColor);
    void fillSelection(KisPixelSelectionSP pixelSelection);

    void setThreshold(int threshold);

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
