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
	m_thumb = 0;
	m_pixmap = 0;

	if (m_resource && m_resource -> valid()) {
		QImage img = m_resource -> img();

		if (img.isNull()) {
			m_resource -> setValid(false);
			m_resource = 0;
			return;
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
				m_thumb = new QPixmap(thumb);
				validThumb = !m_thumb -> isNull();
			}
		}

		img = img.convertDepth(32);
		m_pixmap = new QPixmap(img);
		validPixmap = true;
	}
}

KisIconItem::~KisIconItem()
{
	delete m_thumb;
	delete m_pixmap;
}

QPixmap& KisIconItem::pixmap() const
{
	return *m_pixmap;
}

QPixmap& KisIconItem::thumbPixmap() const
{
	return *m_thumb;
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


int KisIconItem::opacity() const {
	if ( m_resource && m_resource -> valid() ) {
		return m_resource -> opacity();
	}
	else {
		return OPACITY_OPAQUE;
	}
}

void KisIconItem::setOpacity(int opacity) {
	if ( m_resource && m_resource -> valid() ) {
		m_resource -> setOpacity(opacity);
	}
}

int KisIconItem::compositeOp() const {
	if ( m_resource && m_resource -> valid() ) {
		return m_resource -> compositeOp();
	}
	else {
		return COMPOSITE_OVER;
	}
	
}

void KisIconItem::setCompositeOp(int compositeOp) {
	if ( m_resource && m_resource -> valid() ) {
		m_resource -> setCompositeOp((CompositeOp)compositeOp);
	}
}
