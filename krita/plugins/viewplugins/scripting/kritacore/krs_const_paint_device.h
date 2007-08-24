/*
 *  Copyright (c) 2005,2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KROSS_KRITACOREKRSCONSTPAINTDEVICE_H
#define KROSS_KRITACOREKRSCONSTPAINTDEVICE_H

#include <QObject>

#include <kis_types.h>
#include <kis_paint_layer.h>
#include "krosskritacore_export.h"

class KisDoc2;

namespace Scripting {

class Image;

/**
 * A PaintDevice is a layer within a \a Image where you are able
 * to perform paint-operations on.
 */
class KROSSKRITACORE_EXPORT ConstPaintDevice : public QObject
{
        Q_OBJECT
    public:
        explicit ConstPaintDevice(const KisPaintDeviceSP layer, KisDoc2* doc = 0);
        virtual ~ConstPaintDevice();

    public slots:

        /**
         * Return the width of the layer.
         */
        int width();

        /**
         * Return the height of the layer.
         */
        int height();

        /**
         * Return the id of the colorspace of this image (e.g. "RGBA" or "CMYK").
         */
        QString colorSpaceId();

        /**
         * Create an iterator over a layer, it will iterate on a rectangle area.
         * This function takes four arguments :
         *  - x
         *  - y
         *  - width of the rectangle
         *  - height of the rectangle
         */
        QObject* createRectConstIterator(uint x, uint y, uint width, uint height);

        /**
         * Create an iterator over a layer, it will iterate on a row.
         * This function takes three arguments :
         *  - x start in the row
         *  - y vertical position of the row
         *  - width of the row
         */
        QObject* createHLineConstIterator(uint x, uint y, uint width);

        /**
         * Create an iterator over a layer, it will iterate on a column.
         * This function takes three arguments :
         *  - x horizontal position of the column
         *  - y start in the column
         *  - height of the column
         */
        QObject* createVLineConstIterator(uint x, uint y, uint height);

        /**
         * This function creates an Histogram for this layer.
         * It takes two arguments :
         *  - the type of the histogram ("RGB8HISTO")
         *  - 0 if the histogram is linear, or 1 if it is logarithmic
         */
        QObject* createHistogram(const QString& histoname, uint typenr);

        /**
         * Return the fast \a Wavelet transformed of the layer.
         */
        QObject* fastWaveletTransformation();

        /**
         * clone this paint layer, making a deep copy.
         */
        QObject* clone();

    public:
        inline const KisPaintDeviceSP paintDevice() const { return m_device; }
        inline KisDoc2* doc() { return m_doc; }
    private:
        const KisPaintDeviceSP m_device;
        KisDoc2* m_doc;
};

}

#endif
