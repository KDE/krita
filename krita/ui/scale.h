/*
  Gwenview - A simple image viewer for KDE
  Copyright 2000-2004 Aurélien Gâteau

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#ifndef IMAGEUTILS_H
#define IMAGEUTILS_H

// Qt
#include <QImage>

namespace ImageUtils {

    /**
     *
     * SampleImage() scales an image to the desired dimensions with pixel
     * sampling.  Unlike other scaling methods, this method does not introduce
     * any additional color into the scaled image.
     * 
     * The returned image is however only the rect (in coords after scaling) and the actual algorithm
     * does only the work needed to calculate those pixels
     *
     * @param image The image.
     * @param width The width the image is scaled to
     * @param heigh The height the image is scaled to.
     * @param dstRect The rect that is cut out of the scaled image and returned as an image
     *
     */
    QImage sampleImage(const QImage& image, int width, int height, const QRect &dstRect);


    /**
     * smoothscale the given image to the specified width and height.
     *
     * @param image The image.
     * @param width The number of columns in the sampled image.
     * @param heigh The number of rows in the sampled image.
     *
     */
    QImage scale(const QImage& image, int width, int height, Qt::AspectRatioMode mode = Qt::IgnoreAspectRatio);

}
#endif
