/*
 *  Copyright (c) 2004, Boudewijn Rempt <boud@valdyas.org>
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

#include <qobject.h>
#include <qstrlist.h>
#include <qdict.h>

#include "kis_abstract_capability.h"
#include "kis_capability_mediator.h"


KisCapabilityMediator::KisCapabilityMediator() 
{
// 	m_capabilities = new QDict();
}

KisCapabilityMediator::~KisCapabilityMediator() 
{
// 	KisAbstractCapability * tmp;
// 	QDictIterator<KisAbstractCapability> it(m_capabilities);
// 	for ( ; it.current(); ++it) {
// 		tmp = it.current();
// 		m_capabilities.remove(it.currentKey());
// 		delete tmp;
// 		tmp = 0;
// 	}
}

void KisCapabilityMediatorregisterCapability(const QString & name, 
					     const KisAbstractCapability capability)
{
}
	
QStrList* KisCapabilityMediator::capabilities(const QString & filter) const
{
}

KisAbstractCapability * KisCapabilityMediator::getCapability(const QString & name) const
{
}


#include "kis_capability_mediator.moc"
