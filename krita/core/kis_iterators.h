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

#if !defined KIS_ITERATORS_H_
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

/** _Tp is a Unit
	* _Tpu is a KisIteratorUnit
	*  sizeOfTp is the size of unit
	* For instance :
	*
	*/
template< typename _Tp, typename _Tpu, int sizeOfTp = 1 >
class KisIteratorUnit {
	public:
		KisIteratorUnit( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos = 0, Q_INT32 nxpos = 0) : m_device (ndevice),
				m_command (command), m_ktm( m_device->data()),
				m_depth(::imgTypeDepth( m_device->typeWithoutAlpha() ) +1), m_xstart(0), m_ystart(0),
				m_xend( ndevice->width() * ( m_depth ) - 1 ),
				m_yend(ndevice->height() - 1),
				m_ypos(nypos), m_rownum(nypos / TILE_HEIGHT ), m_ypos_intile( nypos % TILE_HEIGHT ),
				m_tilenum( m_ktm->ncols() * m_rownum + nxpos /  TILE_WIDTH ), m_xintile(nxpos % TILE_WIDTH),
				m_tileNeedRefresh (true), m_tileNeedRefreshRW(true)
		{
			kdDebug() << nxpos << " " <<TILE_WIDTH << " " << nxpos /  TILE_WIDTH << " " << m_ktm->ncols() * m_rownum << " " << m_tilenum << endl;
		}
		virtual ~KisIteratorUnit()
		{
		}
	public:
		virtual _Tp operator*() =0;
		virtual  operator _Tp * ()  =0;
//		virtual _Tp* operator->() const = 0;
	//Incrementation operator
		KisIteratorUnit< _Tp, _Tpu, sizeOfTp>& operator++()
		{
			m_xintile+=sizeOfTp;
			if( m_xintile >= m_tile->width() * m_depth )
			{
				m_xintile =  0;
				m_tilenum++;
				m_tileNeedRefresh = true;
				m_tileNeedRefreshRW = true;
			}
			return *this;
		}
		KisIteratorUnit< _Tp, _Tpu, sizeOfTp>& operator--()
		{
			m_xintile-=sizeOfTp;
			if( m_xintile < 0 )
			{
				m_xintile =  m_tile->width() - sizeOfTp;
				m_tilenum--;
				m_tileNeedRefresh = true;
				m_tileNeedRefreshRW = true;
			}
			return *this;
		}
	// Use for debugging purpose
		void print_pos()
		{
			kdDebug() << "m_tilenum=" << m_tilenum << " m_xintile=" << m_xintile << endl;
		}
// Comparaison operators
		bool operator<(const KisIteratorUnit< _Tp, _Tpu, sizeOfTp>& __rhs)
			{ return m_tilenum < __rhs.m_tilenum || ( m_tilenum == __rhs.m_tilenum && m_xintile < __rhs.m_tile ); }
		bool operator<=(const KisIteratorUnit< _Tp, _Tpu, sizeOfTp>& __rhs)
			{
				if( m_tilenum == __rhs.m_tilenum )
				{ return m_xintile <= __rhs.m_xintile; }
				return m_tilenum < __rhs.m_tilenum;
			}
		bool operator==(const KisIteratorUnit< _Tp, _Tpu, sizeOfTp>& __rhs) 
			{ return m_tilenum == __rhs.m_tilenum && m_xintile == __rhs.m_xintile; }

	protected:
		KisPaintDeviceSP m_device;
		KisTileCommand* m_command;
		KisTileMgrSP m_ktm;
		const Q_INT32 m_depth, m_xstart, m_ystart, m_xend, m_yend, m_ypos, m_rownum,m_ypos_intile;
		Q_INT32 m_tilenum, m_xintile;
		bool m_tileNeedRefresh, m_tileNeedRefreshRW;;
		KisTileSP m_tile;
};


/** _iTp is a KisIteratorUnit
	* For instance :
	*   class KisIteratorLineQuantum :
	*      public KisIteratorLine<KisIteratorQuantum>
	*
	*/
template< typename _iTp>
class KisIteratorLine {
	public:
		KisIteratorLine( KisPaintDeviceSP ndevice, KisTileCommand* command, int nypos = 0) : m_device (ndevice), xstart(0), ystart(0),
			xend(ndevice->width()-1), yend(ndevice->height()-1), ypos(nypos), m_command (command)
		{
		}
		virtual ~KisIteratorLine()
		{
		}
	public:
		virtual _iTp operator*()  =0;
		virtual operator _iTp* () =0;
	//Incrementation operator
		KisIteratorLine< _iTp>& operator++() { ypos++; return *this; }
		KisIteratorLine< _iTp>& operator--() { ypos--; return *this; }
// Comparaison operators
		bool operator<(const KisIteratorLine< _iTp>& __rhs)
		{ return this->ypos < __rhs.ypos; }
		bool operator==(const KisIteratorLine< _iTp>& __rhs) 
		{ return this->ypos == __rhs.ypos; }
		bool operator<=(const KisIteratorLine< _iTp>& __rhs)
		{ return this->ypos <= __rhs.ypos; }
		virtual _iTp begin() =0;
		virtual _iTp end() =0;
	protected:
		KisPaintDeviceSP m_device;
		const Q_INT32 xstart, ystart, xend, yend;
		Q_INT32 ypos;
		KisTileCommand* m_command;
};

class KisIteratorQuantum : public KisIteratorUnit<QUANTUM, KisIteratorQuantum,1>
{
	public:
		KisIteratorQuantum( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos = 0, Q_INT32 nxpos = 0);
	public:
		virtual QUANTUM operator*() ;
		virtual operator QUANTUM * ()  ;
//		virtual QUANTUM* operator->() const;
		QUANTUM* m_data;
 ;
};

class KisIteratorLineQuantum : public KisIteratorLine<KisIteratorQuantum>
{
	public:
		KisIteratorLineQuantum( KisPaintDeviceSP ndevice, KisTileCommand* command, int nypos = 0);
	public:
		virtual KisIteratorQuantum operator*() ;
		virtual operator KisIteratorQuantum* ()  ;
		virtual KisIteratorQuantum begin();
		virtual KisIteratorQuantum end();
};

#endif
