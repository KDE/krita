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

#include "kis_iterator.h"


#include <qglobal.h>

#include <kis_debug.h>

#include <config-tiles.h> // For the next define
#include KIS_TILED_ITERATOR_HEADER
#include "kis_datamanager.h"


KisRectConstIterator::KisRectConstIterator(KisDataManager *dm, qint32  x, qint32  y, qint32  w, qint32  h)
{
    m_iter = new KisTiledRectIterator(dm, x, y, w, h, false);
}

KisRectConstIterator::KisRectConstIterator(KisDataManager *dm, qint32  x, qint32  y, qint32  w, qint32  h, bool writable)
{
    m_iter = new KisTiledRectIterator(dm, x, y, w, h, writable);
}

KisRectConstIterator::KisRectConstIterator(const KisRectConstIterator& rhs)
{
    m_iter = rhs.m_iter;
}

KisRectConstIterator& KisRectConstIterator::operator=(const KisRectConstIterator & rhs)
{
    m_iter = rhs.m_iter;
    return *this;
}

KisRectConstIterator::~KisRectConstIterator()
{
}

const quint8 * KisRectConstIterator::rawData() const
{
    return m_iter->rawData();
}

quint8 * KisRectIterator::rawData() const
{
    return m_iter->rawData();
}

const quint8 * KisRectConstIterator::oldRawData() const
{
    return m_iter->oldRawData();
}

qint32 KisRectConstIterator::nConseqPixels() const
{
    return m_iter->nConseqPixels();
}

KisRectConstIterator & KisRectConstIterator::operator+=(int n)
{
    m_iter->operator+=(n); return *this;
}

KisRectConstIterator & KisRectConstIterator::operator++()
{
    m_iter->operator++(); return *this;
}

bool KisRectConstIterator::isDone()  const
{
    return m_iter->isDone();
}

qint32 KisRectConstIterator::x() const
{
    return m_iter->x();
}
qint32 KisRectConstIterator::y() const
{
    return m_iter->y();
}

//---------------------------------------------------------------------------------------

KisHLineConstIterator::KisHLineConstIterator(KisDataManager *dm, qint32  x, qint32 y, qint32 w, bool writable)
{
    m_iter = new KisTiledHLineIterator(dm, x, y, w, writable);
}

KisHLineConstIterator::KisHLineConstIterator(KisDataManager *dm, qint32  x, qint32 y, qint32 w)
{
    m_iter = new KisTiledHLineIterator(dm, x, y, w, false);
}

KisHLineConstIterator::KisHLineConstIterator(const KisHLineConstIterator& rhs)
{
    m_iter = rhs.m_iter;
}

KisHLineConstIterator& KisHLineConstIterator::operator=(const KisHLineConstIterator & rhs)
{

    m_iter = rhs.m_iter;
    return *this;
}

KisHLineConstIterator::~KisHLineConstIterator()
{
}

const quint8 *KisHLineConstIterator::rawData() const
{
    return m_iter->rawData();
}

quint8 *KisHLineIterator::rawData() const
{
    return m_iter->rawData();
}
const quint8 *KisHLineConstIterator::oldRawData() const
{
    return m_iter->oldRawData();
}

KisHLineConstIterator & KisHLineConstIterator::operator++()
{
    m_iter->operator++(); return *this;
}

qint32 KisHLineConstIterator::nConseqHPixels() const
{
    return m_iter->nConseqHPixels();
}

KisHLineConstIterator & KisHLineConstIterator::operator+=(int n)
{
    m_iter->operator+=(n); return *this;
}

KisHLineConstIterator & KisHLineConstIterator::operator--()
{
    m_iter->operator--(); return *this;
}

bool KisHLineConstIterator::isDone()  const
{
    return m_iter->isDone();
}

qint32 KisHLineConstIterator::x() const
{
    return m_iter->x();
}

qint32 KisHLineConstIterator::y() const
{
    return m_iter->y();
}

void KisHLineConstIterator::nextRow()
{
    m_iter->nextRow();
}

//---------------------------------------------------------------------------------------

KisVLineConstIterator::KisVLineConstIterator(KisDataManager *dm, qint32  x, qint32 y, qint32  h, bool writable)
{
    m_iter = new KisTiledVLineIterator(dm, x, y, h, writable);
}

KisVLineConstIterator::KisVLineConstIterator(KisDataManager *dm, qint32  x, qint32 y, qint32  h)
{
    m_iter = new KisTiledVLineIterator(dm, x, y, h, false);
}

KisVLineConstIterator::KisVLineConstIterator(const KisVLineConstIterator& rhs)
{
    m_iter = rhs.m_iter;
}

KisVLineConstIterator& KisVLineConstIterator::operator=(const KisVLineConstIterator & rhs)
{
    m_iter = rhs.m_iter;
    return *this;
}

KisVLineConstIterator::~KisVLineConstIterator()
{
}

const quint8 *KisVLineConstIterator::rawData() const
{
    return m_iter->rawData();
}

quint8 *KisVLineIterator::rawData() const
{
    return m_iter->rawData();
}

const quint8 * KisVLineConstIterator::oldRawData() const
{
    return m_iter->oldRawData();
}

KisVLineConstIterator & KisVLineConstIterator::operator++()
{
    m_iter->operator++(); return *this;
}

bool KisVLineConstIterator::isDone() const
{
    return m_iter->isDone();
}

qint32 KisVLineConstIterator::x() const
{
    return m_iter->x();
}

qint32 KisVLineConstIterator::y() const
{
    return m_iter->y();
}

void KisVLineConstIterator::nextCol()
{
    return m_iter->nextCol();
}
