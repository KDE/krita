/* This file is part of the KDE project
   Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>

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

#ifndef KIS_ITERATORS_INFINITE_H_
#define KIS_ITERATORS_INFINITE_H_

#include "kis_global.h"
#include "kis_types.h"
#include "kis_iterators.h"
#include "kis_iterators_pixel.h"
#include "kis_pixel_representation.h"
#include "color_strategy/kis_strategy_colorspace.h"

/**
 * This iterates over the pixels of a line of a paintdevice as if it was an endless plane
 **/
class KisIteratorInfinitePixel : public KisIteratorPixel
{
public:
	/**
	 * Constructor for an iterator of an 'infinite plane'. The plane argument is the
	 * PaintDevice we'll treat like a plane, for example the result of KisPattern's
	 * image(KisStrategyColorSpaceSP) function, or a gradient (won't work now).
	 * Also note that some parts don't work yet, such as skipPixel();
	 **/
	KisIteratorInfinitePixel( KisPaintDeviceSP plane,
		KisTileCommand* command,
		Q_INT32 nypos = 0,
		Q_INT32 nxpos = 0);
	/**
	 * This is a very special constructor. It takes a KisPixelRepresentation and a 
	 * KisStrategyColorspaceSP as argument, and creates an iterator that only returns
	 * this pixel. Note that it internally creates a new KisLayer, so this might be very
	 * slow. Usage: to be able to switch easily from a pattern to a color.
	 **/
	KisIteratorInfinitePixel(KisStrategyColorSpaceSP s, KisPixelRepresentation p);
public:
	inline virtual KisIteratorInfinitePixel& inc();
	inline virtual KisIteratorInfinitePixel& dec();
	virtual ~KisIteratorInfinitePixel();
private:
	static KisPaintDeviceSP constructPixel(KisStrategyColorSpaceSP s, KisPixelRepresentation p);
	bool m_isPixel;
};


/**
 * This iterates over the lines of a KisPaintDeviceSP as if it was an endless plane. Note
 * that the end() function will only return the end of the 'pattern', since an endless
 * plane doesn't really has an end.
 **/
class KisIteratorInfiniteLinePixel : public KisIteratorLinePixel
{
public:
	/**
	 * Constructor for a lineiterator of a plane. The plane argument is the PaintDevice
	 * we'll treat like a plane, for example the result of KisPattern's
	 * image(KisStrategyColorSpaceSP) function. Because it acts like it's infinite,
	 * there is no nxwidth parameter.
	 **/
	KisIteratorInfiniteLinePixel( KisPaintDeviceSP plane,
		KisTileCommand* command, 
		Q_INT32 nypos = 0,
		Q_INT32 nxstart = -1);
public:
	KisIteratorPixel operator*();
	operator KisIteratorPixel* ();
	inline virtual KisIteratorInfiniteLinePixel& inc();
	inline virtual KisIteratorInfiniteLinePixel& dec();
	virtual KisIteratorPixel begin();
	virtual ~KisIteratorInfiniteLinePixel() {}
private:
	KisPaintDeviceSP m_plane;
};

#endif /* KIS_ITERATORS_INFINITE_H_ */
