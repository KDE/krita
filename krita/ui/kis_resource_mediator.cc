/*
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <QList>

#include <QTableWidgetItem>
#include <KoResourceItemChooser.h>

#include "kdebug.h"

#include "kis_icon_item.h"
#include "KoResource.h"
#include "kis_itemchooser.h"
#include "kis_resourceserver.h"
#include "kis_resource_mediator.h"

KisResourceMediator::KisResourceMediator(KisItemChooser *chooser,
                     QObject *parent,
                     const char *name)
    : QObject(parent), m_chooser(chooser)
{
    setObjectName(name);

    Q_ASSERT(chooser);
    m_activeItem = 0;

    connect(m_chooser, SIGNAL(selected(QTableWidgetItem*)), SLOT(setActiveItem(QTableWidgetItem*)));
}

KisResourceMediator::~KisResourceMediator()
{
}

void KisResourceMediator::connectServer(KisResourceServerBase* rServer)
{
    // Add the initially loaded items
    QList<KoResource*> resources = rServer->resources();

    foreach (KoResource *resource, resources)
        rServerAddedResource(resource);

    // And connect to the server permanently, so that we may receive updates afterwards
    connect(rServer, SIGNAL(resourceAdded(KoResource*)),
            this, SLOT(rServerAddedResource(KoResource*)));
}

KoResource *KisResourceMediator::currentResource() const
{
    if (m_activeItem) {
        Q_ASSERT(dynamic_cast<KoResourceItem*>(m_activeItem));
        return static_cast<KoResourceItem*>(m_activeItem)->resource();
    }

    return 0;
}

KoResourceItem *KisResourceMediator::itemFor(KoResource *r) const
{
    if (m_items.contains(r))
    {
        return m_items[r];
    }
    return 0;
}

KoResource *KisResourceMediator::resourceFor(QTableWidgetItem *item) const
{
    KoResourceItem *kisitem = dynamic_cast<KoResourceItem*>(item);

    return kisitem ? kisitem->resource() : 0;
}

KoResource *KisResourceMediator::resourceFor(KoResourceItem *item) const
{
    return item ? item->resource() : 0;
}

QWidget *KisResourceMediator::chooserWidget() const
{
    return m_chooser;
}

void KisResourceMediator::setActiveItem(QTableWidgetItem *item)
{
    KoResourceItem *kisitem = dynamic_cast<KoResourceItem*>(item);

    if (kisitem) {
        m_activeItem = kisitem;
        m_chooser->setCurrent(item);
        emit activatedResource(kisitem ? kisitem->resource() : 0);
    }
}

void KisResourceMediator::rServerAddedResource(KoResource *resource)
{
    if (resource && resource->valid()) {

        KoResourceItem *item = new KoResourceItem(resource);
        Q_CHECK_PTR(item);

        m_items[resource] = item;

        m_chooser->addItem(item);
        if (m_activeItem == 0) setActiveItem(item);
    }
}

#include "kis_resource_mediator.moc"

