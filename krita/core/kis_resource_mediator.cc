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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <koIconChooser.h>
#include "kis_brush.h"
#include "kis_icon_item.h"
#include "kis_itemchooser.h"
#include "kis_resource.h"
#include "kis_resourceserver.h"
#include "kis_resource_mediator.h"

KisResourceMediator::KisResourceMediator(Q_INT32 mediateOn,
			KisResourceServer *rserver,
			const QString& chooserCaption,
			QWidget *chooserParent,
			const char *chooserName,
			QObject *parent,
			const char *name) : super(parent, name)
{
	Q_ASSERT(rserver);
	m_activeItem = 0;
	m_chooser = new KisItemChooser(true, chooserParent, chooserName);
	connect(m_chooser,
                SIGNAL(selected(KoIconItem*)),
                SLOT(setActiveItem(KoIconItem*)));

	if (mediateOn & MEDIATE_BRUSHES)
		connect(rserver,
                        SIGNAL(loadedBrush(KisBrush*)),
                        this,
                        SLOT(resourceServerLoadedBrush(KisBrush*)));

	m_chooser -> setCaption(chooserCaption);
}

KisResourceMediator::~KisResourceMediator()
{
}

KisResource *KisResourceMediator::currentResource() const
{
	if (m_activeItem) {
		Q_ASSERT(dynamic_cast<KisIconItem*>(m_activeItem));
		return static_cast<KisIconItem*>(m_activeItem) -> resource();
	}

	return 0;
}

KisIconItem *KisResourceMediator::itemFor(KisResource *r) const
{
	return m_items[r];
}

KisResource *KisResourceMediator::resourceFor(KoIconItem *item) const
{
	KisIconItem *kisitem = dynamic_cast<KisIconItem*>(item);

	return kisitem ? kisitem -> resource() : 0;
}

KisResource *KisResourceMediator::resourceFor(KisIconItem *item) const
{
	return item ? item -> resource() : 0;
}

QWidget *KisResourceMediator::chooserWidget() const
{
	return m_chooser;
}

void KisResourceMediator::setActiveItem(KoIconItem *item)
{
	KisIconItem *kisitem = dynamic_cast<KisIconItem*>(item);

	if (kisitem) {
		m_activeItem = kisitem;
		emit activatedResource(kisitem ? kisitem -> resource() : 0);
	}
}

void KisResourceMediator::resourceServerLoadedBrush(KisBrush *brush)
{
	if (brush && brush -> valid()) {
		KisIconItem *item = new KisIconItem(brush);

		m_items[brush] = item;
		item -> setSpacing(brush -> spacing());
		m_chooser -> addItem(item);
		emit addedResource(brush);
	}
}

#include "kis_resource_mediator.moc"

