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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <qmutex.h>
#include "kisscopedlock.h"

KisScopedLock::KisScopedLock(QMutex *lock, bool initialLock)
{
	Q_ASSERT(lock);
	m_mutex = lock;

	if (initialLock)
		m_mutex -> lock();
}

KisScopedLock::~KisScopedLock()
{
	Q_ASSERT(m_mutex);

	if (m_mutex -> locked())
		m_mutex -> unlock();
}

void KisScopedLock::lock()
{
	Q_ASSERT(m_mutex);
	m_mutex -> lock();
}

void KisScopedLock::unlock()
{
	Q_ASSERT(m_mutex);
	m_mutex -> unlock();
}

void KisScopedLock::trylock()
{
	Q_ASSERT(m_mutex);
	m_mutex -> tryLock();
}

