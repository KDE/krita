/*
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.g
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "kis_resource.h"
#include "kis_global.h"
#include "kis_icon_item.h"

#define THUMB_SIZE 30

KisIconItem::KisIconItem(KisResource *resource)
{
	m_resource = resource;
	validPixmap = false;
	validThumb = false;
	updatePixmaps();
}

KisIconItem::~KisIconItem()
{
}

void KisIconItem::updatePixmaps()
{
	validPixmap = false;
	validThumb = false;

	if (m_resource && m_resource -> valid()) {
		QImage img = m_resource -> img();

		if (img.isNull()) {
			m_resource -> setValid(false);
			m_resource = 0;
			return;
		}

		if (m_resource -> hasColor() && m_resource -> useColorAsMask()) {
			img = createColorMaskImage(img);
		}

		if (img.width() > THUMB_SIZE || img.height() > THUMB_SIZE) {
			QImage thumb = img;
			Q_INT32 xsize = THUMB_SIZE;
			Q_INT32 ysize = THUMB_SIZE;
			Q_INT32 picW  = thumb.width();
			Q_INT32 picH  = thumb.height();

			if (picW > picH) {
				float yFactor = (float)((float)(float)picH / (float)picW);

				ysize = (Q_INT32)(yFactor * (float)THUMB_SIZE);
			
				if (ysize > THUMB_SIZE) 
					ysize = THUMB_SIZE;
			} else if (picW < picH) {
				float xFactor = (float)((float)picW / (float)picH);

				xsize = (Q_INT32)(xFactor * (float)THUMB_SIZE);

				if (xsize > THUMB_SIZE) 
					xsize = THUMB_SIZE;
			}

			thumb = thumb.smoothScale(xsize, ysize);

			if (!thumb.isNull()) {
				m_thumb = QPixmap(thumb);
				validThumb = !m_thumb.isNull();
			}
		}

		img = img.convertDepth(32);
		m_pixmap = QPixmap(img);
		validPixmap = true;
	}
}

QImage KisIconItem::createColorMaskImage(QImage srcImage)
{
	QImage image = srcImage;
	image.detach();

	for (int x = 0; x < image.width(); x++) {
		for (int y = 0; y < image.height(); y++) {
			QRgb c = image.pixel(x, y);
			int a = (qGray(c) * qAlpha(c)) / 255;
			image.setPixel(x, y, qRgba(a, 0, a, a));
		}
	}

	return image;
}

QPixmap& KisIconItem::pixmap() const
{
	return const_cast<QPixmap&>(m_pixmap);
}

QPixmap& KisIconItem::thumbPixmap() const
{
	return const_cast<QPixmap&>(m_thumb);
}

KisResource *KisIconItem::resource() const
{
	return m_resource;
}

int KisIconItem::spacing() const {
	if ( m_resource && m_resource -> valid() ) {
		return m_resource -> spacing();
	}
	else {
		return 1;
	}
	
}
void KisIconItem::setSpacing(int spacing) {
	if ( m_resource && m_resource -> valid() ) {
		m_resource -> setSpacing(spacing);
	}
}

bool KisIconItem::useColorAsMask() const {
	if ( m_resource && m_resource -> valid() ) {
		return m_resource -> useColorAsMask();
	}
	else {
		return false;
	}
}

void KisIconItem::setUseColorAsMask(bool useColorAsMask) {
	if ( m_resource && m_resource -> valid() ) {
		m_resource -> setUseColorAsMask(useColorAsMask);
		updatePixmaps();
	}
}

bool KisIconItem::hasColor() const
{
	if ( m_resource && m_resource -> valid() ) {
		return m_resource -> hasColor();
	}
	else {
		return false;
	}
}

