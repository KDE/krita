/*
 *  copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#ifndef KIS_PAINT_DEVICE_H_
#define KIS_PAINT_DEVICE_H_

class QImage;
#include "qrect.h"
#include "qglobal.h"
/**
 * Class modelled on QPaintDevice.
 */
class KisPaintDevice {

public:

    KisPaintDevice();
    virtual ~KisPaintDevice();
    
    /**
     * Returns the inexact extent of this paint device.
     *
     * Implementations may optimize by returning not the exact extent,
     * but a rectangle that is guaranteed to contain the exact extent
     * of the paint device, and thus can be bigger.
     */
    virtual QRect extent() const = 0;

    /**
     * Get the exact bounds of this paint device. This may be very slow in some
     * implementations, especially on larger paint devices because it does a 
     * linear scanline search.
     */
    virtual QRect exactBounds() = 0;
        
    /**
     * Completely erase the current paint device. Its size will become 0.
     */
    virtual void clear() = 0;
    
    /**
     * Read the bytes representing the rectangle described by x, y, w, h into
     * data. If data is not big enough, Krita will gladly overwrite the rest
     * of your precious memory.
     *
     * Since this is a copy, you need to make sure you have enough memory.
     *
     * Reading from areas not previously initialized will read the default
     * pixel value into data.
     */
    virtual void readBytes(Q_UINT8 * data, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h) = 0;

    /**
     * Copy the bytes in data into the rect specified by x, y, w, h. If there
     * data is too small or uninitialized, Krita will happily read parts of
     * memory you never wanted to be read.
     *
     * If the data is written to areas of the paint device not previously initialized,
     * the paint device will grow.
     */
    virtual void writeBytes(const Q_UINT8 * data, Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h) = 0;

    /**
     * Fill this paint device with the data from img; starting at (offsetX, offsetY)
     *
     * @param img The QImage to convert from. We will use the RGBA colorspace and no profile.
     * @param offsetX the offset in X coordinates
     * @param offsetY the offset in Y coordinates
     */
    virtual void convertFromQImage(const QImage& img, Q_INT32 offsetX = 0, Q_INT32 offsetY = 0) = 0;

    /**
     * Return the number of bytes a pixel takes.
     */
    virtual Q_INT32 pixelSize() const = 0;

    /**
     * Return the number of channels a pixel takes
     */
    virtual Q_INT32 nChannels() const = 0;



private:

    KisPaintDevice(const KisPaintDevice&);
    KisPaintDevice& operator=(const KisPaintDevice&);

};


#endif // KIS_PAINT_DEVICE_H_

