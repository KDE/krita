/*
 *  Copyright (c) 2000 Matthias Elter  <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien  <freak@codepimps.org>
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

#include <qpainter.h>
#include <koIconChooser.h>
#include "kis_iconwidget.h"

KisIconWidget::KisIconWidget(QWidget *parent, const char *name) : super(parent, name)
{
    m_item = 0;
}

void KisIconWidget::slotSetItem(KoIconItem& item)
{
    m_item = &item;
    update();
}

void KisIconWidget::drawButtonLabel(QPainter *p)
{
    if (m_item) {
        const QPixmap& pix = m_item->pixmap();
        Q_INT32 x = 2;
        Q_INT32 y = 2;
        Q_INT32 pw = pix.width();
        Q_INT32 ph = pix.height();
        Q_INT32 cw = width();
        Q_INT32 ch = height();
        Q_INT32 itemWidth = 24;
        Q_INT32 itemHeight = 24;

        if (pw < itemWidth)
            x = (cw - pw) / 2;
        if (ph < itemHeight)
            y = (cw - ph) / 2;

        if (!m_item->hasValidThumb() || (pw <= itemWidth && ph <= itemHeight)) {
            p->drawPixmap(x, y, pix, 0, 0, itemWidth, itemHeight);
        } else {
            const QPixmap& thumbpix = m_item->thumbPixmap();

            x = 2;
            y = 2;
            pw = thumbpix.width();
            ph = thumbpix.height();
            cw = width();
            ch = height();

            if (pw < itemWidth)
                x = (cw - pw) / 2;

            if (ph < itemHeight)
                y = (cw - ph) / 2;

            p->drawPixmap(x, y, thumbpix, 0, 0, itemWidth, itemHeight);
        }

        p->setPen(gray);
        p->drawRect(0, 0, cw + 1, ch + 1);
    }
}

#include "kis_iconwidget.moc"

