/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef _KIS_PIXEL_H_
#define _KIS_PIXEL_H_

#include "kis_global.h"
#include "kis_types.h"

#include "kis_quantum.h"

// XXX: Template these classes to byte, int, float, double
// XXX: Separate the alpha channel for the other channels on construction.
// XXX: Separate color channels from substance channels (wetness, thickness, grainyness) on construction

/**
 * KisPixel and KisPixelRO are the primary class to access individual pixel data.
 * A pixel consists of channels of a certain size. At the moment, all channels must
 * be of the same size, and that size is one byte, or QUANTUM. In the future
 * pixels with heteregenous channels and larger (or smaller?) channels will be
 * possible, too.
 *
 * The [] operator returns the channel at that index. You can discover the number
 * of channels and the names of the channels by querying the color strategy.
 * All channels can be accessed using this method.
 *
 * There are three types of channels: color, alpha and substance.
 *
 * Generally, in filters and suchlike code, you can treat all color channels the same, 
 * and have a different algorithm for alpha. Substance is a future extension, usable
 * for wetness, grainyness or thickness.
 *
 * The color channels come first -- this is useful if you want to loop over 
 * these channels (for int i = 0; i < colorspace -> nColorChannels(); ++i) { hack(pixel[i]); }.
 *
 * The next index after the colour channels points to the alpha channel. A pointer to
 * the value of the alpha channel can also be retrieved with alpha(). Always
 * check whether there is actually an alpha channel, otherwise alpha() will
 * return the first channel of the next pixel, most probably, or perhaps even
 * something worse.
 * 
 * After alpha, the substance channels will come.
 */


/**
 * A read-only pixel. You can retrieve the channel values by name or position
 * or all channels as a value vector.
 */
class KisPixelRO {
public:
	KisPixelRO() 
		: m_channels(0) {}

	KisPixelRO(QUANTUM* channels) 
		: m_channels(channels) {}

	KisPixelRO(QUANTUM* channels, QUANTUM* alpha)
		: m_channels(channels), m_alpha(alpha) {}


public:

	QUANTUM operator[](int index) { return m_channels[index]; }

	QUANTUM alpha() { return m_alpha[0]; }
	KisStrategyColorSpaceSP colorStrategy() { return m_colorStrategy; }

private:
	QUANTUM* m_channels;
	QUANTUM* m_alpha;
	KisStrategyColorSpaceSP m_colorStrategy;
};


/**
 * A read-write pixel. You can retrieve the channel values by name or position
 * or all channels as a pointer vector.
 */
class KisPixel {

public:

	KisPixel(QUANTUM* channels) : m_channels(channels) {}

	KisPixel(int nbchannel) : m_channels(new QUANTUM(nbchannel)) { }

	KisPixel(QUANTUM* channels, QUANTUM* alpha)
		: m_channels(channels), m_alpha(alpha) {}

public:

	KisQuantum operator[](int index) { return KisQuantum(&m_channels[index]); };
	KisQuantum alpha() { return KisQuantum(m_alpha); };
	KisStrategyColorSpaceSP colorStrategy() { return m_colorStrategy; };

private:
	QUANTUM* m_channels;
	QUANTUM* m_alpha;
	KisStrategyColorSpaceSP m_colorStrategy;
};



#endif
