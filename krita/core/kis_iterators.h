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

#ifndef KIS_ITERATORS_H_
#define KIS_ITERATORS_H_

#include <qstring.h>
#include "kis_types.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include "kis_tile_command.h"
#include "kis_undo_adapter.h"
#include "tiles/kis_iterator.h"
#include <kdebug.h>
#include "kis_quantum.h"
#include "kis_pixel.h"

/** 
 * These classe can be used to iterate over the pixel data in
 * a given paint device, reading and writing data. 
 *
 * Do not use this class directly. Instead, use one of the descendant 
 * classes: KisIteratorPixel or KisIteratorQuantum.
 */
class KisIteratorUnit {

public:
	KisIteratorUnit( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos, Q_INT32 nxpos, Q_INT8 inc);

public:
	//Increment operator
	inline KisIteratorUnit& operator++();
	inline virtual KisIteratorUnit& inc();
	inline virtual KisIteratorUnit& dec();
	inline KisIteratorUnit& operator--();

	/**
	 * Increment the position of the iterator by one pixel.
	 */

	inline void skipPixel();

	// Comparison operators
	inline bool operator<(const KisIteratorUnit& __rhs) const;
	inline bool operator<=(const KisIteratorUnit& __rhs) const;
	inline bool operator==(const KisIteratorUnit& __rhs) const;

	//Data access operators
	inline operator KisPixel();
	inline KisPixel value();
	inline KisPixelRO oldValue();
	inline KisQuantum operator[](int index);

	virtual ~KisIteratorUnit() {}

protected:
	inline operator QUANTUM () ;
	inline operator QUANTUM*();
	inline QUANTUM* oldQuantumValue();

protected:
	KisPaintDeviceSP m_device;
	KisTileCommand* m_command;
	KisStrategyColorSpaceSP m_colorSpace;
	KisHLineIterator m_underlying_iterator;
	QUANTUM* m_data;
	QUANTUM* m_oldData;
	Q_INT32 m_x;
};


/**
 * _iTp is a KisIteratorUnit
 * For instance :
 *   class KisIteratorLineQuantum :
 *      public KisIteratorLine<KisIteratorQuantum>
 */
template< typename _iTp> 
class KisIteratorLine {
public:
	KisIteratorLine( KisPaintDeviceSP ndevice, 
			 KisTileCommand* command, 
			 Q_INT32 nypos,
			 Q_INT32 nxstart, 
			 Q_INT32 nxend) :
		m_device( ndevice ),
		m_xstart(nxstart),
		m_xend(nxend),
		m_ypos(nypos), m_command( command )
		{
		}

	virtual ~KisIteratorLine()
		{
		}

public:
	virtual _iTp operator*()  = 0;
	virtual operator _iTp* () = 0;

	//Increment operator
	KisIteratorLine< _iTp>& operator++() { return inc(); }
	KisIteratorLine< _iTp>& operator--() { return dec(); }

	//Overridable operator functionality
	inline virtual KisIteratorLine< _iTp>& inc() { m_ypos++; return *this; }
	inline virtual KisIteratorLine< _iTp>& dec() { m_ypos--; return *this; }

	// Comparison operators
	bool operator<(const KisIteratorLine< _iTp>& __rhs) const
		{ 
			return this->m_ypos < __rhs.m_ypos; 
		}

	bool operator==(const KisIteratorLine< _iTp>& __rhs) const 
		{ 
			return this->m_ypos == __rhs.m_ypos; 
		}

	bool operator<=(const KisIteratorLine< _iTp>& __rhs) const
		{ 
			return this->m_ypos <= __rhs.m_ypos; 
		}

	virtual _iTp begin() =0;
	virtual _iTp end() =0;

protected:
	KisPaintDeviceSP m_device;
	const Q_INT32 m_xstart, m_xend;
	Q_INT32 m_ypos;
	KisTileCommand* m_command;
};

inline KisPixelRO KisIteratorUnit::oldValue()
{
	return m_colorSpace -> toKisPixelRO( this->oldQuantumValue(), m_device -> profile());
}

inline KisIteratorUnit::operator KisPixel()
{
	return m_colorSpace -> toKisPixel((QUANTUM*)(*this), m_device -> profile());
}

inline KisPixel KisIteratorUnit::value()
{
	return *this;
}

inline KisQuantum KisIteratorUnit::operator[](int index)
{
	return m_colorSpace -> toKisPixel((QUANTUM*)(*this), m_device -> profile())[index];
}

inline KisIteratorUnit& KisIteratorUnit::operator++()
{
	return inc();
}
inline KisIteratorUnit& KisIteratorUnit::inc()
{
	m_underlying_iterator++;
	m_x++;
	return *this;
}

inline KisIteratorUnit& KisIteratorUnit::dec()
{
	m_underlying_iterator--;
	m_x--;
	return *this;
}

inline KisIteratorUnit& KisIteratorUnit::operator--()
{
	return dec();
}
/**
 * This function increments the position of the iterator by one pixel.
 */
inline void KisIteratorUnit::skipPixel() 
{
	inc();
}
// Comparison operators
inline bool KisIteratorUnit::operator<(const KisIteratorUnit& __rhs) const
{ 
	return m_x < __rhs.m_x; 
}
inline bool KisIteratorUnit::operator<=(const KisIteratorUnit& __rhs) const
{
	return m_x < __rhs.m_x;
}
inline bool KisIteratorUnit::operator==(const KisIteratorUnit& __rhs) const 
{ 
	return m_x == __rhs.m_x; 
}

inline KisIteratorUnit::operator QUANTUM*()
{
	return (Q_UINT8 *)m_underlying_iterator;
}


inline KisIteratorUnit::operator QUANTUM ()
{
	return *((Q_UINT8 *)m_underlying_iterator);
}

inline QUANTUM* KisIteratorUnit::oldQuantumValue()
{
#if 0 // AUTOLAYER
	if( m_oldTileNeedRefresh )
	{
		m_oldTile = 0;
		if( m_command)
		{
			m_oldTile = (m_command->tile(m_tilenum));
		}
		if( m_oldTile == 0)
		{
			if( !(m_oldTile = (m_ktm->tile( m_tilenum, TILEMODE_READ) ) ) )
				return 0;
		}
		m_oldData =  m_oldTile->data(0, m_ypos_intile);
		m_oldTileNeedRefresh = false;
	}
	return (m_oldData + m_xintile);
#endif  // AUTOLAYER
return 0;
}

#endif
