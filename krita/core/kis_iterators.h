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

template< typename _Tp, int sizeOfTp = 1 >
class KisIteratorPixel {
	public:
		KisIteratorPixel( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos) : m_device (ndevice),
			 xstart(0), ystart(0), xend( ndevice->width() * (::imgTypeDepth( m_device->typeWithoutAlpha() ) +1 ) ),
			 yend(ndevice->height()), ypos(nypos), m_command (command)
		{
		}
		virtual ~KisIteratorPixel()
		{
		}
	public:
		virtual _Tp operator*() const = 0;
		virtual _Tp* operator->() const = 0;
	//Incrementation operator
		KisIteratorPixel< _Tp, sizeOfTp>& operator++() { xpos+sizeOfTp; return *this; }
		KisIteratorPixel< _Tp, sizeOfTp>& operator--() { xpos-sizeOfTp; return *this; }
// Comparaison operators
	inline bool operator<(const KisIteratorPixel< _Tp, sizeOfTp>& __rhs)
	{ return this->xpos < __rhs.xpos; }
	inline bool operator==(const KisIteratorPixel< _Tp, sizeOfTp>& __rhs) 
	{ return this->xpos == __rhs.xpos; }
	
	KisIteratorPixel< _Tp, sizeOfTp> begin() {
		KisIteratorPixel< _Tp, sizeOfTp> it( m_device, m_command, ypos);
		it.xpos = it.xstart;
	}
	KisIteratorPixel< _Tp, sizeOfTp> end() {
		KisIteratorPixel< _Tp, sizeOfTp> it( m_device, m_command, ypos);
		it.xpos = it.xend;
	}
	protected:
		KisPaintDeviceSP m_device;
		const Q_INT32 xstart, ystart, xend, yend, ypos;
		Q_INT32 xpos;
		KisTileCommand* m_command;
};



template< typename _iTp>
class KisIteratorLine {
	public:
		KisIteratorLine( KisPaintDeviceSP ndevice, KisTileCommand* command) : m_device (ndevice), xstart(0), ystart(0),
			xend(ndevice->width()), yend(ndevice->height()), ypos(0), m_command (command)
		{
		}
		virtual ~KisIteratorLine()
		{
		}
	public:
		virtual _iTp operator*() const =0;
		virtual _iTp* operator->() const =0;
	//Incrementation operator
		KisIteratorLine< _iTp>& operator++() { ypos++; return *this; }
		KisIteratorLine< _iTp>& operator--() { ypos--; return *this; }
// Comparaison operators
	inline bool operator<(const KisIteratorLine< _iTp>& __rhs)
	{ return this->ypos < __rhs.ypos; }
	inline bool operator==(const KisIteratorLine< _iTp>& __rhs) 
	{ return this->ypos == __rhs.ypos; }
	
	KisIteratorLine< _iTp> begin() {
		KisIteratorLine< _iTp> it( m_device, m_command);
		it.ypos = it.ystart;
	}
	KisIteratorLine< _iTp> end() {
		KisIteratorLine< _iTp> it( m_device, m_command);
		it.ypos = it.yend;
	}
	protected:
		KisPaintDeviceSP m_device;
		const Q_INT32 xstart, ystart, xend, yend;
		Q_INT32 ypos;
		KisTileCommand* m_command;
};

class KisIteratorPixelQuantum : public KisIteratorPixel<QUANTUM, 1>
{
	public:
		KisIteratorPixelQuantum( KisPaintDeviceSP ndevice, KisTileCommand* command, Q_INT32 nypos);
	public:
		virtual QUANTUM operator*() const;
		virtual QUANTUM* operator->() const;
};

class KisIteratorLineQuantum : public KisIteratorLine<KisIteratorPixelQuantum>
{
	public:
		KisIteratorLineQuantum( KisPaintDeviceSP ndevice, KisTileCommand* command);
	public:
		virtual KisIteratorPixelQuantum operator*() const;
		virtual KisIteratorPixelQuantum* operator->() const;
};

#endif
