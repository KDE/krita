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
#include "kis_tile_command.h"
#include "kis_memento.h"

KisTileCommand::KisTileCommand(const QString& name, KisPaintDeviceSP device,
			       Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height)
{
	m_name = name;
	m_device = device;
	m_rc.setRect(x + m_device->getX(), y + m_device->getY(), width, height);
	m_memento = device -> getMemento();
}

KisTileCommand::KisTileCommand(const QString& name, KisPaintDeviceSP device, const QRect& rc)
{
	m_name = name;
	m_device = device;
	m_rc = rc;
	m_rc.moveBy(m_device->getX(), m_device->getY());
	m_memento = device -> getMemento();
}

KisTileCommand::KisTileCommand(const QString& name, KisPaintDeviceSP device)
{
	m_name = name;
	m_device = device;
	//AUTOLAYER m_rc = device -> bounds();
	m_rc.setRect(0, 0, 10, 10);
	m_rc.moveBy(m_device->getX(), m_device->getY());
	m_memento = device -> getMemento();
}

KisTileCommand::~KisTileCommand()
{
	delete m_memento;
}

void KisTileCommand::execute()
{
	KisImageSP img = m_device -> image();
	
	m_device->rollforward(m_memento);
	
	if (img)
		img -> notify(m_rc);
}

void KisTileCommand::unexecute()
{
	KisImageSP img = m_device -> image();
	
	m_device -> rollback(m_memento);
	
	if (img)
		img -> notify(m_rc);
}

QString KisTileCommand::name() const
{
	return m_name;
}
