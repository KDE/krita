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

#include "kis_resource_mediator.h"

#include <QList>
#include <QTableWidgetItem>

#include <KoResource.h>
#include <KoResourceItemChooser.h>
#include <KoResourceServerAdapter.h>

#include "kis_itemchooser.h"

KisResourceMediator::KisResourceMediator(KisItemChooser *chooser,
                     KoAbstractResourceServerAdapter* rServerAdapter,
                     QObject *parent,
                     const char *name)
    : QObject(parent), m_chooser(chooser), m_rServerAdapter(rServerAdapter)
{
    setObjectName(name);

    Q_ASSERT(chooser);
    Q_ASSERT(rServerAdapter);

    connect(m_chooser, SIGNAL(selected(QTableWidgetItem*)), SLOT(setActiveItem(QTableWidgetItem*)));
    connect(m_chooser, SIGNAL(deleteClicked() ), this, SLOT( deleteActiveResource() ) );

    connect(m_rServerAdapter, SIGNAL(resourceAdded(KoResource*) ), this, SLOT( rServerAddedResource(KoResource*) ) );
    connect(m_rServerAdapter, SIGNAL(removingResource(KoResource*) ), this, SLOT( rServerRemovingResource(KoResource*) ) );
    m_rServerAdapter->connectToResourceServer();
}

KisResourceMediator::~KisResourceMediator()
{
}

KoResource *KisResourceMediator::currentResource() const
{
    if(! m_chooser->currentItem() )
        return 0;

    Q_ASSERT(dynamic_cast<KoResourceItem*>(m_chooser->currentItem()));
    return static_cast<KoResourceItem*>(m_chooser->currentItem())->resource();
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
        m_chooser->setCurrent(item);
    }
    emit activatedResource(kisitem ? kisitem->resource() : 0);
}

void KisResourceMediator::deleteActiveResource()
{
    KoResource* r = currentResource();
    if(!itemFor(r))
        return;

    m_rServerAdapter->removeResource(r);
}

void KisResourceMediator::addResourceItem(KoResourceItem* item)
{
    if (item->resource() && item->resource()->valid()) {

        m_items[item->resource()] = item;

        m_chooser->addItem(item);
        if (m_chooser->currentItem() == 0) setActiveItem(item);
    }
}

void KisResourceMediator::rServerAddedResource(KoResource *resource)
{
    if (resource && resource->valid()) {

        KoResourceItem *item = new KoResourceItem(resource);
        Q_CHECK_PTR(item);

        addResourceItem(item);
    }
}

void KisResourceMediator::rServerRemovingResource(KoResource *resource)
{
    KoResourceItem *item = itemFor(resource);
    if(item)
        removeResourceItem(item);
}

void KisResourceMediator::removeResourceItem(KoResourceItem* item)
{
    m_items.remove(item->resource());
    m_chooser->removeItem(item);

    setActiveItem(m_chooser->currentItem());
}

#include "kis_resource_mediator.moc"

