/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#if !defined KIS_ALPHA_MASK_
#define KIS_ALPHA_MASK_

#include <qimage.h>
#include <qvaluevector.h>

#include <ksharedptr.h>

#include "kis_global.h"

class KisAlphaMask : public KShared {
	
 public:
	/**
	   Create an alpha mask based on the gray values of the
	   specified QImage. If the QImage is not grayscale, you're
	   buggered.
	*/
	KisAlphaMask(const QImage& img);

	/**
	   Create an alpha mask based on the gray values of the
	   specified QImage. If the QImage is not grayscale, you're
	   buggered. The QImage is scaled using QImage::smoothScale,
	   where the target w and h are computed by taking scale as a
	   percentage.
	*/
	KisAlphaMask(const QImage& img, double scale);

	/**
	   Create a transparent mask.
	*/
	KisAlphaMask(Q_INT32 width, Q_INT32 height, double scale);

	virtual ~KisAlphaMask();

	/**
	   @return the number of alpha values in a scanline.
	*/
	Q_INT32 height() const;

	/**
	   @return the number of lines in the mask.
	 */
   	Q_INT32 width() const;

	/**
	   @return the scale factor.
	*/
	double scale() const;
	
	/**
	   @return the alpha value at the specified position.

	   Returns QUANTUM OPACITY_TRANSPARENT if the value is
	   outside the bounds of the mask.

	   XXX: this is, of course, not the best way of masking.
	   Better would be to let KisAlphaMask fill a chunk of memory
	   with the alpha values at the right position, something like
	   void applyMask(QUANTUM *pixeldata, Q_INT32 pixelWidth,
	   Q_INT32 alphaPos). That would be fastest, or we could
	   provide an iterator over the mask, that would be nice, too.
	*/
	QUANTUM alphaAt(Q_INT32 x, Q_INT32 y) const;

	void setAlphaAt(Q_INT32 x, Q_INT32 y, QUANTUM alpha);

private:

	void computeAlpha(const QImage& img);
	void copyAlpha(const QImage& img);

	QValueVector<QUANTUM> m_data;
	double m_scale;
	Q_INT32 m_scaledWidth;
	Q_INT32 m_scaledHeight;

};

#endif // KIS_ALPHA_MASK_

