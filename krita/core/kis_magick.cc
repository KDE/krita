/*
 *  kis_magick.cc - part of Krita aka Krayon aka KImageShop
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qcolor.h>
#include <qimage.h>

#include "kis_magick.h"

using namespace Magick;

QImage convertFromMagickImage(Image& mimg)
{
	QImage img;
	Geometry geo = mimg.size();
	const PixelPacket *pixel_cache;

	if (geo.width() && geo.height()) {
		img.create(geo.width(), geo.height(), 32);
		pixel_cache = mimg.getConstPixels(0, 0, mimg.columns(), mimg.rows());

		for (int row = 0; row < img.height(); row++) {
			for (int column = 0; column < img.width(); column++) {
				const PixelPacket *pixel = pixel_cache + row * img.width() + column;
				QRgb rgb;

#if QuantumDepth == 8
				rgb = qRgba(pixel -> red, pixel -> green, pixel -> blue, pixel -> opacity);
#elif QuantumDepth == 16
				rgb = qRgba(Downscale(pixel -> red), Downscale(pixel -> green), Downscale(pixel -> blue), Downscale(pixel -> opacity));
#else
#error "Unkown QuantumDepth"
#endif

				img.setPixel(column, row, rgb);
			}
		}
	}

	return img;
}

Image convertToMagickImage(QImage& img)
{
	Image mimg;
	PixelPacket *pixel_cache;
	Geometry geo(img.width(), img.height());

	mimg.size(geo);
	pixel_cache = mimg.getPixels(0, 0, mimg.columns(), mimg.rows());

	for (int row = 0; row < img.height(); row++) {
		for (int column = 0; column < img.width(); column++) {
			PixelPacket *pixel = pixel_cache + row * img.width() + column;
			QRgb rgb = img.pixel(column, row);

#if QuantumDepth == 8
			*pixel = Color(qRed(rgb), qGreen(rgb), qBlue(rgb), qAlpha(rgb));
#elif QuantumDepth == 16
			*pixel = Color(Upscale(qRed(rgb)), Upscale(qGreen(rgb)), Upscale(qBlue(rgb)), Upscale(qAlpha(rgb)));
#else
#error "Unkown QuantumDepth"
#endif
		}
	}

	mimg.syncPixels();
	return mimg;
}

