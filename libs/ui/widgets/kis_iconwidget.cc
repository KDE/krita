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

#include "widgets/kis_iconwidget.h"

#include <QPainter>
#include <QIcon>
#include <QStyleOption>
#include <resources/KoResource.h>
#include <KoResourceServerAdapter.h>


KisIconWidget::KisIconWidget(QWidget *parent, const char *name)
    : KisPopupButton(parent)
{
    setObjectName(name);
    m_resource = 0;
}

void KisIconWidget::setResource(KoResource * resource)
{
    m_resource = resource;
    update();
}

void KisIconWidget::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter p;
    p.begin(this);

    const qint32 cw = width();
    const qint32 ch = height();
    const qint32 border = 3;
    const qint32 iconWidth = cw - (border*2);
    const qint32 iconHeight = ch - (border*2);

    // Round off the corners of the preview
    QRegion clipRegion(border, border, iconWidth, iconHeight);
    clipRegion -= QRegion(border, border, 1, 1);
    clipRegion -= QRegion(cw-border-1, border, 1, 1);
    clipRegion -= QRegion(cw-border-1, ch-border-1, 1, 1);
    clipRegion -= QRegion(border, ch-border-1, 1, 1);

    p.setClipRegion(clipRegion);
    p.setClipping(true);

    p.setBrush(this->palette().background());
    p.drawRect(QRect(0,0,cw,ch));

    if (m_resource) {
        QImage img = QImage(iconWidth, iconHeight, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        if (m_resource->image().width()<iconWidth ||
                m_resource->image().height()<iconHeight) {
            QPainter paint2;
            paint2.begin(&img);
            for (int x=0; x< iconWidth; x+=m_resource->image().width()) {
                for (int y=0; y< iconHeight; y+=m_resource->image().height()) {
                    paint2.drawImage(x, y, m_resource->image());
                }
            }
        } else {
            img = m_resource->image().scaled(iconWidth, iconHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        p.drawImage(QRect(border, border, iconWidth, iconHeight), img);
    } else if (!icon().isNull()) {
        int border2 = qRound((cw-16)*0.5);
        p.drawImage(QRect(border2, border2, 16, 16), icon().pixmap(16, 16).toImage());
    }
    p.setClipping(false);
}

