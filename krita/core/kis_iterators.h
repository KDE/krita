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
		KisIteratorUnit( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos) : m_device (ndevice),
			 xstart(0), ystart(0), xend( ndevice->width() * (::imgTypeDepth( m_device->typeWithoutAlpha() ) +1 ) - 1 ),
			 yend(ndevice->height() - 1), xpos(0),ypos(nypos), m_command (command)
		{
		}
		virtual ~KisIteratorUnit()
		{
		}
	public:
		virtual _Tp operator*() const =0;
		virtual  operator _Tp * ()  const =0;
//		virtual _Tp* operator->() const = 0;
	//Incrementation operator
		KisIteratorUnit< _Tp, _Tpu, sizeOfTp>& operator++()  { xpos+=sizeOfTp; return *this; }
		KisIteratorUnit< _Tp, _Tpu, sizeOfTp>& operator--() { xpos-=sizeOfTp; return *this; }
// Comparaison operators
	bool operator<(const KisIteratorUnit< _Tp, _Tpu, sizeOfTp>& __rhs)
	{ return this->xpos < __rhs.xpos; }
	bool operator<=(const KisIteratorUnit< _Tp, _Tpu, sizeOfTp>& __rhs)
	{ return this->xpos <= __rhs.xpos; }
	bool operator==(const KisIteratorUnit< _Tp, _Tpu, sizeOfTp>& __rhs) 
	{ return this->xpos == __rhs.xpos; }
	
	_Tpu begin() {
		_Tpu it = createBlankIterator();
		it.xpos = it.xstart;
		return it;
	}
	_Tpu end() {
		_Tpu it =createBlankIterator();
		it.xpos = it.xend;
		return it;
	}
	protected:
		/** This function is used to create a new _Tpu
			*/
		virtual _Tpu createBlankIterator() =0;
	protected:
		KisPaintDeviceSP m_device;
		const Q_INT32 xstart, ystart, xend, yend, ypos;
		Q_INT32 xpos;
		KisTileCommand* m_command;
};


/** _iTp is a KisIteratorUnit
	* _iTl is a KisIteratorLine
	* For instance :
	*   class KisIteratorLineQuantum :
	*      public KisIteratorLine<KisIteratorQuantum, KisIteratorLineQuantum>
	*
	*/
template< typename _iTp, typename _iTl>
class KisIteratorLine {
	public:
		KisIteratorLine( KisPaintDeviceSP ndevice, KisTileCommand* command) : m_device (ndevice), xstart(0), ystart(0),
			xend(ndevice->width()-1), yend(ndevice->height()-1), ypos(0), m_command (command)
		{
		}
		virtual ~KisIteratorLine()
		{
		}
	public:
		virtual _iTp operator*() const =0;
		virtual operator _iTp ()  const =0;
	//Incrementation operator
		KisIteratorLine< _iTp, _iTl>& operator++() { ypos++; return *this; }
		KisIteratorLine< _iTp, _iTl>& operator--() { ypos--; return *this; }
// Comparaison operators
	bool operator<(const KisIteratorLine< _iTp, _iTl>& __rhs)
	{ return this->ypos < __rhs.ypos; }
	bool operator==(const KisIteratorLine< _iTp, _iTl>& __rhs) 
	{ return this->ypos == __rhs.ypos; }
	bool operator<=(const KisIteratorLine< _iTp, _iTl>& __rhs)
	{ return this->ypos <= __rhs.ypos; }
	_iTl begin() {
		_iTl it = createBlankIterator();
		it.ypos = it.ystart;
		return it;
	}
	_iTl end() {
		_iTl it = createBlankIterator();
		it.ypos = it.yend;
		return it;
	}
	protected:
		/** This function is used to create a new _iTl
			*/
		virtual _iTl createBlankIterator() =0;
	protected:
		KisPaintDeviceSP m_device;
		const Q_INT32 xstart, ystart, xend, yend;
		Q_INT32 ypos;
		KisTileCommand* m_command;
};

class KisIteratorQuantum : public KisIteratorUnit<QUANTUM, KisIteratorQuantum,1>
{
	public:
		KisIteratorQuantum( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos);
	public:
		virtual QUANTUM operator*() const;
		virtual operator QUANTUM * ()  const;
//		virtual QUANTUM* operator->() const;
	protected:
		virtual inline KisIteratorQuantum createBlankIterator() {
			return KisIteratorQuantum(m_device, m_command, ypos);
		}
 ;
};

class KisIteratorLineQuantum : public KisIteratorLine<KisIteratorQuantum, KisIteratorLineQuantum>
{
	public:
		KisIteratorLineQuantum( KisPaintDeviceSP ndevice, KisTileCommand* command);
	public:
		virtual KisIteratorQuantum operator*() const;
		virtual operator KisIteratorQuantum ()  const;
	protected:
		virtual inline KisIteratorLineQuantum createBlankIterator() {
			return KisIteratorLineQuantum(m_device, m_command);
		} ;
};

#endif
