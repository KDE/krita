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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kdebug.h>

#include "kis_resource.h"
#include "kis_global.h"
#include "kis_icon_item.h"
//Added by qt3to4:
#include <QPixmap>

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

    if (m_resource && m_resource->valid()) {
        QImage img = m_resource->img();

        if (img.isNull()) {
            m_resource->setValid(false);
            m_resource = 0;
            return;
        }

        if (img.width() > THUMB_SIZE || img.height() > THUMB_SIZE) {
            QImage thumb = img;
            qint32 xsize = THUMB_SIZE;
            qint32 ysize = THUMB_SIZE;
            qint32 picW  = thumb.width();
            qint32 picH  = thumb.height();

            if (picW > picH) {
                float yFactor = (float)((float)(float)picH / (float)picW);

                ysize = (qint32)(yFactor * (float)THUMB_SIZE);
            
                if (ysize > THUMB_SIZE) 
                    ysize = THUMB_SIZE;
            } else if (picW < picH) {
                float xFactor = (float)((float)picW / (float)picH);

                xsize = (qint32)(xFactor * (float)THUMB_SIZE);

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

int KisIconItem::compare(const KoIconItem *o) const
{
    const KisIconItem *other = dynamic_cast<const KisIconItem *>(o);

    if (other != 0) {
        return m_resource->name().localeAwareCompare(other->m_resource->name());
    } else {
        return 0;
    }
}

