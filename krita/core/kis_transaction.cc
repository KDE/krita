/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include "kis_types.h"
#include "kis_global.h"
#include "tiles/kis_tile.h"
#include "tiles/kis_tileddatamanager.h"
#include "kis_image.h"
#include "kis_transaction.h"
#include "kis_memento.h"

KisTransaction::KisTransaction(const QString& name, KisPaintDeviceSP device)
{
	m_name = name;
	m_device = device;
	m_memento = device -> getMemento();
}

KisTransaction::~KisTransaction()
{
	delete m_memento;
}

void KisTransaction::execute()
{
	KisImageSP img = m_device -> image();
	
	m_device->rollforward(m_memento);
	
	QRect rc;	
	Q_INT32 x, y, width, height;
	m_memento->extent(x,y,width,height);
	rc.setRect(x + m_device->getX(), y + m_device->getY(), width, height);
	if (img)
		img -> notify(rc);
}

void KisTransaction::unexecute()
{
	KisImageSP img = m_device -> image();
	
	m_device -> rollback(m_memento);
	
	QRect rc;	
	Q_INT32 x, y, width, height;
	m_memento->extent(x,y,width,height);
	rc.setRect(x + m_device->getX(), y + m_device->getY(), width, height);
	if (img)
		img -> notify(rc);
}

QString KisTransaction::name() const
{
	return m_name;
}
