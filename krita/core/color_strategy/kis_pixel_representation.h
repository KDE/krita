/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "kis_global.h"
#include "kis_quantum.h"

#ifndef _KIS_PIXEL_REPRESENTATION_H_
#define _KIS_PIXEL_REPRESENTATION_H_

class KisPixelRepresentationReadOnly {
public:
	KisPixelRepresentationReadOnly() : m_channels(0) { }
	KisPixelRepresentationReadOnly(QUANTUM* channels) : m_channels(channels) {}
public:
	QUANTUM operator[](int index) { return m_channels[index]; };
private:
	QUANTUM* m_channels;
};


class KisPixelRepresentation {
public:
	KisPixelRepresentation(QUANTUM* channels) : m_channels(channels) {}
	KisPixelRepresentation(int nbchannel) : m_channels(new QUANTUM(nbchannel)) { }
public:
	KisQuantum operator[](int index) { return KisQuantum(&m_channels[index]); };
private:
	QUANTUM* m_channels;
};

// XXX: conversions always via koColor; this class is BAD. RGB is unsuited as 
// an intermediary colourspace format, and besides, we must use koColor and later
// littleCMS for that.
class KisPixelRepresentationRGB : public KisPixelRepresentation {
public:
	inline KisPixelRepresentationRGB( const KisPixelRepresentation& pr) : KisPixelRepresentation(pr) { };
	inline KisPixelRepresentationRGB( ) : KisPixelRepresentation(4) { };
public:
	inline KisQuantum red() { return (*this)[PIXEL_RED]; };
	inline KisQuantum green() { return (*this)[PIXEL_GREEN]; };
	inline KisQuantum blue() { return (*this)[PIXEL_BLUE]; };
	inline KisQuantum alpha() { return (*this)[PIXEL_ALPHA]; };
};


#endif
