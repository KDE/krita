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
#include <qptrlist.h>
 
#include <koIconChooser.h>

#include "kdebug.h"

#include "kis_brush.h"
#include "kis_pattern.h"
#include "kis_gradient.h"

#include "kis_icon_item.h"
#include "kis_brush_chooser.h"
#include "kis_pattern_chooser.h"
#include "kis_gradient_chooser.h"
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

	QPtrList<KisResource> resouceslist;
	KisResource *resource;

	if (mediateOn & MEDIATE_BRUSHES) {
		m_chooser = new KisBrushChooser(chooserParent, chooserName);
		connect(rserver,
			SIGNAL(loadedBrush(KisResource*)),
			this,
			SLOT(resourceServerLoadedResource(KisResource*)));

		connect(rserver,
			SIGNAL(loadedpipeBrush(KisResource*)),
			this,
			SLOT(resourceServerLoadedResource(KisResource*)));

		resouceslist = rserver -> brushes();
		for ( resource = resouceslist.first(); resource; resource = resouceslist.next() )
			resourceServerLoadedResource(resource);

		resouceslist = rserver -> pipebrushes();
		for ( resource = resouceslist.first(); resource; resource = resouceslist.next() )
			resourceServerLoadedResource(resource);
	}
	if (mediateOn & MEDIATE_PATTERNS) {
		m_chooser = new KisPatternChooser(chooserParent, chooserName);
		connect(rserver,
			SIGNAL(loadedPattern(KisResource*)),
			this,
			SLOT(resourceServerLoadedResource(KisResource*)));

		resouceslist = rserver -> patterns();
		for ( resource = resouceslist.first(); resource; resource = resouceslist.next() )
			resourceServerLoadedResource(resource);
			
	}
	if (mediateOn & MEDIATE_GRADIENTS) {
		m_chooser = new KisGradientChooser(chooserParent, chooserName);
		connect(rserver,
			SIGNAL(loadedGradient(KisResource*)),
			this,
			SLOT(resourceServerLoadedResource(KisResource*)));

		resouceslist = rserver -> gradients();
		for ( resource = resouceslist.first(); resource; resource = resouceslist.next() )
			resourceServerLoadedResource(resource);
	}

	connect(m_chooser, SIGNAL(selected(KoIconItem*)), SLOT(setActiveItem(KoIconItem*)));
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
	if(m_items.contains(r))
	{
		return m_items[r];
	}
	return 0;
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
		m_chooser -> setCurrent(item);
		emit activatedResource(kisitem ? kisitem -> resource() : 0);
	}
}

void KisResourceMediator::resourceServerLoadedResource(KisResource *resource)
{
	if (resource && resource -> valid()) {
		
		KisIconItem *item = new KisIconItem(resource);

		m_items[resource] = item;

		m_chooser -> addItem(item);
		emit addedResource(resource);
		if (m_activeItem == 0) setActiveItem(item);
	}
}

#include "kis_resource_mediator.moc"

