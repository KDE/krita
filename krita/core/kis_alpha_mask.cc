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

#include <qimage.h>
#include <qvaluevector.h>

#include <kdebug.h>

#include "kis_global.h"
#include "kis_alpha_mask.h"

KisAlphaMask::KisAlphaMask(const QImage& img) 
{
	m_scale = 1;
	computeAlpha(img);
}

KisAlphaMask::KisAlphaMask(const QImage& img, double scale)
{
	m_scale = scale;
	QImage scaledImg = img.smoothScale((int)(img.width() * scale), 
					   (int)(img.height() * scale));
	computeAlpha(scaledImg);
}

KisAlphaMask::KisAlphaMask(Q_INT32 width, Q_INT32 height, double scale)
{
	m_scale = scale;
	m_scaledWidth = width;
	m_scaledHeight = height;
	m_data.resize(width * height, OPACITY_TRANSPARENT);
}

KisAlphaMask::~KisAlphaMask() 
{}

Q_INT32 KisAlphaMask::width() const
{
	return m_scaledWidth;
}

Q_INT32 KisAlphaMask::height() const
{
	return m_scaledHeight;
}

double KisAlphaMask::scale() const
{
	return m_scale;
}

QUANTUM KisAlphaMask::alphaAt(Q_INT32 x, Q_INT32 y) const
{
	if (y >= 0 && y < m_scaledHeight && x >= 0 && x < m_scaledWidth) {
		return m_data[((y) * m_scaledWidth) + x];
	}
	else {
		kdDebug() << "oops \n";
		return OPACITY_TRANSPARENT;
	}
}

void KisAlphaMask::setAlphaAt(Q_INT32 x, Q_INT32 y, QUANTUM alpha)
{
	if (y >= 0 && y < m_scaledHeight && x >= 0 && x < m_scaledWidth) {
		m_data[((y) * m_scaledWidth) + x] = alpha;
	}
	else {
		kdDebug() << "oops \n";
	}
}

void KisAlphaMask::copyAlpha(const QImage& img) 
{
	m_scaledWidth = img.width();
	m_scaledHeight = img.height();
	for (int y = 0; y < img.height(); y++) {
		for (int x = 0; x < img.width(); x++) {
			// Wish it were this simple: this makes a mask, like the Gimp uses, but I like my
			// own solution better, and so do my kids
			// m_data.push_back(255 - qAlpha(img.pixel(x,y)));
                        QRgb c = img.pixel(x,y);
                        QUANTUM a = (qGray(c) * qAlpha(c)) / 255;
			m_data.push_back(a);

		}
	}
}

void KisAlphaMask::computeAlpha(const QImage& img) 
{
	m_scaledWidth = img.width();
	m_scaledHeight = img.height();

	// The brushes are mostly grayscale on a white background,
	// although some do have a colors. The alpha channel is seldom
	// used, so we take the average gray value of this pixel of
	// the brush as the setting for the opacitiy. We need to
	// invert it, because 255, 255, 255 is white, which is
	// completely transparent, but 255 corresponds to
	// OPACITY_OPAQUE.
	//
	// If the alpha value is not 255, or the r,g and b values are
	// not the same, we have a real coloured brush, and are
	// knackered for the nonce.

	if (!img.allGray()) {
		copyAlpha(img);
	}
	else {
		// All gray -- any colour is alpha mask
		for (int y = 0; y < img.height(); y++) {
			for (int x = 0; x < img.width(); x++) {
				m_data.push_back (255 - qRed(img.pixel(x,y)));
			}
			
		}
	}
	
}
