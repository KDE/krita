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

#ifndef KROSS_KRITACOREKRSIMAGE_H
#define KROSS_KRITACOREKRSIMAGE_H

#include <QObject>

#include <kis_types.h>

class KisDoc;

namespace Kross {

namespace KritaCore {

class Image : public QObject
{
       //Q_OBJECT
    public:
        Image(KisImageSP image, KisDoc* doc = 0);
        ~Image();

    //public slots:

#if 0
        /**
         * Return the active PaintLayer, if any.
         */
        Kross::Api::Object::Ptr getActivePaintLayer(Kross::Api::List::Ptr);
        /**
         * Return the width of the image.
         */
        Kross::Api::Object::Ptr getWidth(Kross::Api::List::Ptr);
        /**
         * Return the height of the image.
         */
        Kross::Api::Object::Ptr getHeight(Kross::Api::List::Ptr);
        /**
         * Resize an image
         */
        Kross::Api::Object::Ptr resize(Kross::Api::List::Ptr);
        /**
         * Scale an image
         */
        Kross::Api::Object::Ptr scale(Kross::Api::List::Ptr);
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
         * Return the id of the colorspace of this image.
         */
        Kross::Api::Object::Ptr colorSpaceId(Kross::Api::List::Ptr );
        /**
         * Create a new PaintLayer for this image, and return it.
         * This function takes at least two arguments :
         *  - the name of the layer
         *  - the opacity of the layer (between 0 and 255)
         * 
         * This function can take one optional argument :
         *  - the id of the colorSpace (if this is not specified, the new PaintLayer
         *      will have the same colorspace as the image)
         */
        Kross::Api::Object::Ptr createPaintLayer(Kross::Api::List::Ptr args);
#endif

    private:
        KisImageSP m_image;
        KisDoc* m_doc;
};

}

}

#endif
