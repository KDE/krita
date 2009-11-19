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
#include <KoResource.h>


KisIconWidget::KisIconWidget(QWidget *parent, const char *name)
        : KisPopupButton(parent)
{
    setObjectName(name);
    m_resource = 0;
    setFixedSize(QSize(26, 26));
}

void KisIconWidget::slotSetItem(KoResource * resource)
{
    m_resource = resource;
    update();
}

void KisIconWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    qint32 cw = width();
    qint32 ch = height();
    if (m_resource) {
        p.drawImage(QRect(0, 0, 24, 24), m_resource->img());
    } else {
        p.fillRect(0, 0, cw, ch, Qt::white);
    }
    p.setPen(Qt::gray);
    p.drawRect(0, 0, cw + 1, ch + 1);
    (void)p.end();
    paintPopupArrow();
}

#include "kis_iconwidget.moc"

