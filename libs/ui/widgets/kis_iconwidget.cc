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

#include <QStylePainter>
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

void KisIconWidget::slotSetItem(KoResource * resource)
{
    m_resource = resource;
    update();
}

void KisIconWidget::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QStylePainter p(this);

    const qint32 cw = width();
    const qint32 ch = height();
    const qint32 border = 1;
    const qint32 iconWidth = cw - (border*2);
    const qint32 iconHeight = ch - (border*2);

    QRegion clipRegion(border, border, iconWidth, iconHeight);
    clipRegion -= QRegion(border,border,1,1);
    clipRegion -= QRegion(cw-(border*2),border,1,1);
    clipRegion -= QRegion(cw-(border*2),ch-(border*2),1,1);
    clipRegion -= QRegion(border,ch-(border*2),1,1);

    p.setClipRegion(clipRegion);
    p.setClipping(true);

    p.setBrush(Qt::white);
    p.drawRect(QRect(0,0,cw,ch));

    if (m_resource) {
        p.drawImage(QRect(border, border, iconWidth, iconHeight), m_resource->image());
    }

    p.setClipping(false);
}

void KisIconWidget::setResourceAdapter(QSharedPointer<KoAbstractResourceServerAdapter> adapter)
{
    Q_ASSERT(adapter);
    adapter->connectToResourceServer();
    connect(adapter.data(), SIGNAL(resourceChanged(KoResource*)), this, SLOT(slotAdapterResourceChanged(KoResource*)));
}

void KisIconWidget::slotAdapterResourceChanged(KoResource* resource)
{
    if (m_resource == resource) {
        update();
    }
}
