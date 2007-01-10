// vim: set tabstop=4 shiftwidth=4 noexpandtab
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
#include <qimage.h>

// Local
#include "imageutils/orientation.h"

namespace ImageUtils {
	enum SmoothAlgorithm { SMOOTH_NONE, SMOOTH_FAST, SMOOTH_NORMAL, SMOOTH_BEST };

	QImage scale(const QImage& image, int width, int height,
		SmoothAlgorithm alg, QImage::ScaleMode mode = QImage::ScaleFree, double blur = 1.0);

	int extraScalePixels( SmoothAlgorithm alg, double zoom, double blur = 1.0 );

	QImage transform(const QImage& img, Orientation orientation);

	QImage changeBrightness( const QImage& image, int brightness );

	QImage changeContrast( const QImage& image, int contrast );

	QImage changeGamma( const QImage& image, int gamma );

	QWMatrix transformMatrix(Orientation orientation);
}

#endif
