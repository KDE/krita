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
#include <kistile.h>
#include <kistilemgr.h>
#include <kdebug.h>

/** 
 * These classe can be used to iterate over the pixel data in
 * a given paint device, reading and writing data. 
 *
 * Do not use this class directly. Instead, use one of the descendant 
 * classes: KisIteratorPixel or KisIteratorQuantum.
 */
class KisIteratorUnit {

public:
	KisIteratorUnit( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos = 0, Q_INT32 nxpos = 0, Q_INT8 inc = 1);

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
	inline operator QUANTUM () ;
	inline operator QUANTUM*();
	inline QUANTUM* oldQuantumValue();

	virtual ~KisIteratorUnit() {}

protected:
	KisPaintDeviceSP m_device;
	KisTileCommand* m_command;
	KisTileMgrSP m_ktm;
	const Q_INT32 m_depth, m_ypos, m_rownum, m_ypos_intile;
	Q_INT32 m_tilenum, m_xintile;
	bool m_oldTileNeedRefresh, m_tileNeedRefresh, m_tileNeedRefreshRW;
	KisTileSP m_tile, m_oldTile;
	Q_INT8 m_inc;
	QUANTUM* m_data;
	QUANTUM* m_oldData;
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
			 Q_INT32 nypos = 0,
			 Q_INT32 nxstart = -1, 
			 Q_INT32 nxend = -1) :
		m_device( ndevice ),
		m_xstart( (nxstart < 0) ? 0 : nxstart  ),
		m_xend( ( nxend < 0 ) ? ndevice->width()-1 : nxend ),
		m_ypos( nypos ), m_command( command )
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


inline KisIteratorUnit& KisIteratorUnit::operator++()
{
	return inc();
}
inline KisIteratorUnit& KisIteratorUnit::inc()
{
	Q_ASSERT( m_tile != 0 );
	m_xintile+= m_inc;
	if( m_xintile >= m_tile->width() * m_depth )
		{
		m_xintile =  0;
		m_tilenum++;
		m_tileNeedRefresh = true;
		m_tileNeedRefreshRW = true;
		m_oldTileNeedRefresh = true;
	}
	return *this;
}

inline KisIteratorUnit& KisIteratorUnit::dec()
{
	Q_ASSERT( m_tile != 0 );
	m_xintile-=m_inc;
	if( m_xintile < 0 )
	{
		m_xintile =  m_tile->width() * m_depth - m_inc;
		m_tilenum--;
		m_tileNeedRefresh = true;
		m_tileNeedRefreshRW = true;
		m_oldTileNeedRefresh = true;
	}
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
	Q_ASSERT( m_tile != 0 );
	m_xintile += m_depth;
	if( m_xintile >= m_tile->width() * m_depth )
	{
		m_xintile -= m_tile->width() * m_depth;
		m_tilenum++;
		m_tileNeedRefresh = true;
		m_tileNeedRefreshRW = true;
	}
}
// Comparison operators
inline bool KisIteratorUnit::operator<(const KisIteratorUnit& __rhs) const
{ 
	return m_tilenum < __rhs.m_tilenum || (m_tilenum == __rhs.m_tilenum && m_xintile < __rhs.m_xintile); 
}
inline bool KisIteratorUnit::operator<=(const KisIteratorUnit& __rhs) const
{
	if( m_tilenum == __rhs.m_tilenum ) { 
		return m_xintile <= __rhs.m_xintile; 
	}
	
	return m_tilenum < __rhs.m_tilenum;
}
inline bool KisIteratorUnit::operator==(const KisIteratorUnit& __rhs) const 
{ 
	return m_tilenum == __rhs.m_tilenum && m_xintile == __rhs.m_xintile; 
}

inline KisIteratorUnit::operator QUANTUM*()
{
	if( m_tileNeedRefreshRW )
	{
		if( m_command && (m_tile = m_ktm->tile( m_tilenum , TILEMODE_NONE)) )
		{
			m_command->addTile( m_tilenum , m_tile);
		}
		if (!(m_tile = m_ktm->tile( m_tilenum, TILEMODE_RW)))
			return 0;
		m_data =  m_tile->data(0, m_ypos_intile);
		m_tileNeedRefreshRW = false;
	}
	return m_data + m_xintile;
}


inline KisIteratorUnit::operator QUANTUM ()
{
	if( m_tileNeedRefresh && m_tileNeedRefreshRW )
	{
		if( !(m_tile = (m_ktm->tile( m_tilenum, TILEMODE_READ) ) ) )
		{
			return 0;
		}
		m_data =  m_tile->data(0, m_ypos_intile);
		m_tileNeedRefresh = false;
	}
	return *(m_data + m_xintile);
}

inline QUANTUM* KisIteratorUnit::oldQuantumValue()
{
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
}

#endif
