/* This file is part of the KDE project
   Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/


#if !defined KIS_ITERATORS_QUANTUM_H_
#define KIS_ITERATORS_QUANTUM_H_

#include <kdebug.h>
#include "kis_iterators.h"
#include "kis_quantum.h"

/**
 * XXX: document
 */
class KisIteratorQuantum : public KisIteratorUnit
{
public:
	KisIteratorQuantum( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos = 0, Q_INT32 nxpos = 0);
public:
	using KisIteratorUnit::operator QUANTUM;
	operator KisQuantum ()  ;
	QUANTUM operator=(QUANTUM q);
};


/**
 * XXX: document
 */
class KisIteratorLineQuantum : public KisIteratorLine<KisIteratorQuantum>
{
public:
	KisIteratorLineQuantum( KisPaintDeviceSP ndevice, 
				KisTileCommand* command, 
				Q_INT32 nypos = 0,
				Q_INT32 nxstart = -1, 
				Q_INT32 nxend = -1);
public:
	virtual KisIteratorQuantum operator*();
	virtual operator KisIteratorQuantum* ();
	virtual KisIteratorQuantum begin();
	virtual KisIteratorQuantum end();
};

inline QUANTUM KisIteratorQuantum::operator=(QUANTUM q)
{
	KisQuantum kq = *this;
	return ( kq = q);
}

inline KisIteratorQuantum::operator KisQuantum ()
{
	return KisQuantum((QUANTUM*)*this);
}

#endif
