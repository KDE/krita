/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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

#ifndef KROSS_KRITACOREKRSLAYER_H
#define KROSS_KRITACOREKRSLAYER_H

#include <QObject>

#include <kis_types.h>
#include <kis_paint_layer.h>

class KisDoc;
class KisTransaction;

namespace Kross {

namespace KritaCore {

/**
@author Cyrille Berger
*/
class PaintLayer : public QObject
{
        //Q_OBJECT
    public:
        explicit PaintLayer(KisPaintLayerSP layer, KisDoc* doc = 0);
        virtual ~PaintLayer();

    //public slots:

#if 0
        /**
         * Create an iterator over a layer, it will iterate on a rectangle area.
         * This function takes four arguments :
         *  - x
         *  - y
         *  - width of the rectangle
         *  - height of the rectangle
         */
        Kross::Api::Object::Ptr createRectIterator(Kross::Api::List::Ptr);
        /**
         * Create an iterator over a layer, it will iterate on a row.
         * This function takes three arguments :
         *  - x start in the row
         *  - y vertical position of the row
         *  - width of the row
         */
        Kross::Api::Object::Ptr createHLineIterator(Kross::Api::List::Ptr);
        /**
         * Create an iterator over a layer, it will iterate on a column.
         * This function takes three arguments :
         *  - x horizontal position of the column
         *  - y start in the column
         *  - height of the column
         */
        Kross::Api::Object::Ptr createVLineIterator(Kross::Api::List::Ptr);
        /**
         * Return the width of the layer
         */
        Kross::Api::Object::Ptr getWidth(Kross::Api::List::Ptr);
        /**
         * Return the height of the layer
         */
        Kross::Api::Object::Ptr getHeight(Kross::Api::List::Ptr);
        /**
         * This function creates an Histogram for this layer.
         * It takes two arguments :
         *  - the type of the histogram ("RGB8HISTO")
         *  - 0 if the histogram is linear, or 1 if it is logarithmic
         */
        Kross::Api::Object::Ptr createHistogram(Kross::Api::List::Ptr);
        /**
         * This function create a Painter which will allow you to some painting on the layer.
         */
        Kross::Api::Object::Ptr createPainter(Kross::Api::List::Ptr);
        /**
         * Uses this function to create a new undo entry.
         */
        Kross::Api::Object::Ptr beginPainting(Kross::Api::List::Ptr args);
        /**
         * Uses this function to close the current undo entry and add it to the history.
         */
        Kross::Api::Object::Ptr endPainting(Kross::Api::List::Ptr args);
        /**
         * Convert the image to a colorspace.
         * This function takes one argument :
         *  - the name of the destination colorspace
         * 
         * For example (in Ruby) :
         * @code
         * image.convertToColorspace("CMYK")
         * @endcode
         */
        Kross::Api::Object::Ptr convertToColorspace(Kross::Api::List::Ptr args);
        /**
         * Return the id of the colorspace of this paint layer.
         */
        Kross::Api::Object::Ptr colorSpaceId(Kross::Api::List::Ptr );
        /**
         * Return the fast wavelet transformed of the layer
         */
        Kross::Api::Object::Ptr fastWaveletTransformation(Kross::Api::List::Ptr args);
        /**
         * Untransform a fast wavelet into this layer
         * It takes one argument :
         *  - a wavelet object
         * 
         * For example (in Ruby) :
         * @code
         * wavelet = layer.fastWaveletTransformation()
         * layer.fastWaveletUntransformation(wavelet)
         * @endcode
         */
        Kross::Api::Object::Ptr fastWaveletUntransformation(Kross::Api::List::Ptr args);
#endif

    public:
        inline KisPaintLayerSP paintLayer() { return m_layer; }
        inline KisDoc* doc() { return m_doc; }
    private:
        KisPaintLayerSP m_layer;
        KisDoc* m_doc;
        KisTransaction* m_cmd;
};

}

}

#endif
