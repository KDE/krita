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
	KisQuantum operator[](int index) { return KisQuantum(m_channels + index ); };
private:
	QUANTUM* m_channels;
};

#endif
