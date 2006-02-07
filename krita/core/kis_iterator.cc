/* This file is part of the KDE project
 *   Copyright (c) 2004 Casper Boemann <cbr@boemann.dkt>
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


#include <qglobal.h>

#include <kdebug.h>

#include "kis_iterator.h"
#include "kis_datamanager.h"
#include "kis_tilediterator.h"

KisRectIterator::KisRectIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32  y, Q_INT32  w, Q_INT32  h, bool writable) 
{
    m_iter = new KisTiledRectIterator(dm, x, y, w, h, writable);
}
KisRectIterator::KisRectIterator(const KisRectIterator& rhs) 
{
    m_iter = rhs.m_iter;
}

KisRectIterator& KisRectIterator::operator=(const KisRectIterator& rhs)
{
    m_iter = rhs.m_iter;
    return *this;
}

KisRectIterator::~KisRectIterator()
{
}

Q_UINT8 * KisRectIterator::rawData() const { return m_iter->rawData();}

const Q_UINT8 * KisRectIterator::oldRawData() const { return m_iter->oldRawData();}

Q_INT32 KisRectIterator::nConseqPixels() const { return m_iter->nConseqPixels(); }

KisRectIterator & KisRectIterator::operator+=(int n) { m_iter->operator+=(n); return *this; }

KisRectIterator & KisRectIterator::operator++() { m_iter->operator++(); return *this; }

bool KisRectIterator::isDone()  const { return m_iter->isDone(); }

Q_INT32 KisRectIterator::x() const { return m_iter->x(); }
Q_INT32 KisRectIterator::y() const { return m_iter->y(); }

//---------------------------------------------------------------------------------------

KisHLineIterator::KisHLineIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32 y, Q_INT32 w, bool writable) 
{
    m_iter = new KisTiledHLineIterator(dm, x, y, w, writable);
}

KisHLineIterator::KisHLineIterator(const KisHLineIterator& rhs)
{
    m_iter = rhs.m_iter;
}

KisHLineIterator& KisHLineIterator::operator=(const KisHLineIterator& rhs)
{ 

    m_iter=rhs.m_iter; 
    return *this; 
}

KisHLineIterator::~KisHLineIterator()
{
}

Q_UINT8 *KisHLineIterator::rawData() const 
{ 
    return m_iter->rawData();
}

const Q_UINT8 *KisHLineIterator::oldRawData() const { return m_iter->oldRawData();}

KisHLineIterator & KisHLineIterator::operator++() { m_iter->operator++(); return *this; }

Q_INT32 KisHLineIterator::nConseqHPixels() const { return m_iter->nConseqHPixels(); }

KisHLineIterator & KisHLineIterator::operator+=(int n) { m_iter->operator+=(n); return *this; }

KisHLineIterator & KisHLineIterator::operator--() { m_iter->operator--(); return *this; }

bool KisHLineIterator::isDone()  const { return m_iter->isDone(); }

Q_INT32 KisHLineIterator::x() const { return m_iter->x(); }
 
Q_INT32 KisHLineIterator::y() const { return m_iter->y(); }

void KisHLineIterator::nextRow() { m_iter->nextRow(); }

//---------------------------------------------------------------------------------------

KisVLineIterator::KisVLineIterator ( KisDataManager *dm, Q_INT32  x, Q_INT32 y, Q_INT32  h, bool writable)
{
    m_iter = new KisTiledVLineIterator(dm, x, y, h, writable);
}

KisVLineIterator::KisVLineIterator(const KisVLineIterator& rhs)
{
    m_iter = rhs.m_iter;
}

KisVLineIterator& KisVLineIterator::operator=(const KisVLineIterator& rhs)
{
    m_iter = rhs.m_iter;
    return *this; 
}

KisVLineIterator::~KisVLineIterator()
{
}

Q_UINT8 *KisVLineIterator::rawData() const { return m_iter->rawData();}

const Q_UINT8 * KisVLineIterator::oldRawData() const { return m_iter->oldRawData();}

KisVLineIterator & KisVLineIterator::operator++() { m_iter->operator++(); return *this; }

bool KisVLineIterator::isDone() const { return m_iter->isDone(); }

Q_INT32 KisVLineIterator::x() const { return m_iter->x(); }

Q_INT32 KisVLineIterator::y() const { return m_iter->y(); }

void KisVLineIterator::nextCol() { return m_iter->nextCol(); }
