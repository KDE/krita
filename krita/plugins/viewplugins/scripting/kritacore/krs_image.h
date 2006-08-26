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
       Q_OBJECT
    public:
        Image(KisImageSP image, KisDoc* doc = 0);
        ~Image();

    public slots:

        /**
         * Return the active \a PaintLayer or NULL if there is
         * no PaintLayer active yet.
         */
        QObject* activePaintLayer();

        /**
         * Return the width of the image.
         */
        int width() const;

        /**
         * Return the height of the image.
         */
        int height() const;

        /**
         * Return the id of the colorspace of this image (e.g. "RGBA" or "CMYK").
         */
        QString colorSpaceId() const;

        /**
         * Convert the image to a colorspace.
         * This function takes one argument :
         *  - the name of the destination colorspace
         * This function returns true if convert to the
         * colorspace was successfully else (e.g. if the
         * colorspace is not available, please check your
         * installation in that case) false is returned.
         *
         * For example (in Ruby) :
         * @code
         * # set the colorspace to "CMYK"
         * image.convertToColorspace("CMYK")
         * # following line will print "CMYK" now.
         * image.colorSpaceId()
         * @endcode
         */
        bool convertToColorspace(const QString& colorspacename);

        /**
         * Resize the image.
         * This function takes four arguments :
         *  - the new width.
         *  - the new height.
         *  - x-position (if you don't need this, set it to 0).
         *  - y-position (if you don't need this, set it to 0).
         */
        void resize(int width, int height, int x, int y);

        /**
         * Scale an image.
         * This function takes two arguments :
         *  - the width scalefactor.
         *  - the height scalefactor.
         */
        void scale(double widthfactor, double heightfactor);

        /**
         * Rotate the image.
         * This function takes one argument :
         *  - the angle.
         */
        void rotate(double angle);

        /**
         * Shear the image.
         * This function takes two arguments :
         *  - the X-angle.
         *  - the Y-angle.
         */
        void shear(double xangle, double yangle);

#if 0
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
