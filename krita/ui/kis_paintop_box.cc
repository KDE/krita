/*
 *  kis_paintop_box.cc - part of KImageShop/Krayon/Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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
#include <qwidget.h>
#include <qstring.h>
#include <qvaluelist.h>

#include <kis_id.h>
#include <kis_paintop_registry.h>
#include <kis_view.h>

#include "kis_paintop_box.h"

KisPaintopBox::KisPaintopBox (KisView * parent, const char * name, WFlags f)
	: super (parent, name, f),
	  m_view(parent)
{
	m_paintops = new QValueList<KisID>();

	connect(this, SIGNAL(selected(const KisID &)), m_view, SLOT(paintopActivated(const KisID &)));
	connect(this, SIGNAL(selected(int)), this, SLOT(slotItemselected(int)));

	// XXX: Let's see... Are all paintops loaded and ready?
	KisIDList keys = KisPaintOpRegistry::instance()->listKeys();
	for ( KisIDList::Iterator it = keys.begin(); it != keys.end(); ++it ) {
		addItem(*it);
	}

}
	
KisPaintopBox::~KisPaintopBox()
{
	delete m_paintops;
}

void KisPaintopBox::addItem(const KisID & paintop, const QString & /*category*/)
{
	m_paintops->append(paintop);
	insertItem(paintop.name());
}

void KisPaintopBox::slotItemSelected(int index)
{
	KisID id = *m_paintops->at(index);
	emit selected(id);
}

#include "kis_paintop_box.moc"
