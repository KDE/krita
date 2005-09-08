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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef _KIS_PIXEL_H_
#define _KIS_PIXEL_H_

#include "ksharedptr.h"

#include "kis_global.h"
#include "kis_types.h"
#include "kis_profile.h"
#include "kis_quantum.h"
#include "kis_abstract_colorspace.h"

class QColor;

// XXX: Template these classes to byte, int, float, double
// XXX: Separate color channels from substance channels (wetness, thickness, grainyness) on construction

/**
 * KisPixel and KisPixelRO are the primary classes to access individual pixel data.
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
 *
 * Note: once you start working with color management a sequence of bytes that happens to
 * encode R, G and B values is still meaningless. You cannot know what _color_ the bytes
 * represent unless you also have a profile. So KisPixel must know about the profile, too.
 */


/**
 * A read-only pixel. You can retrieve the channel values by name or position
 * or all channels as a value vector.
 */

class KisPixelRO {
public:

        KisPixelRO(const QUANTUM * channels = 0, const QUANTUM* alpha = 0, KisAbstractColorSpace * colorSpace = 0, KisProfileSP profile = 0)
                : m_channels(channels),
                  m_alpha(alpha),
                  m_colorSpace(colorSpace),
                  m_profile(profile) {};

    virtual ~KisPixelRO() {}

    
public:

    QUANTUM operator[](int index) const { return m_channels[index]; }

    QUANTUM alpha() const { return m_alpha[0]; }

    KisAbstractColorSpace * colorSpace() const { return m_colorSpace; }

    void setProfile(KisProfileSP profile) { m_profile = profile; }

    KisProfileSP profile() const { return m_profile; }

private:
    const QUANTUM* m_channels;
    const QUANTUM* m_alpha;
    KisAbstractColorSpace * m_colorSpace;
    KisProfileSP m_profile;
};


/**
 * A read-write pixel. You can retrieve the channel values by name or position
 * or all channels as a pointer vector.
 *
 * The alpha channel is separately available.
 */
class KisPixel {

public:

    /**
     * Create a new pixel with the specified number of channels and alpha channels.
     */
    KisPixel(int nbChannels, int nbAlphaChannels = 1, KisAbstractColorSpace * colorSpace = 0, KisProfileSP profile = 0) 

        : m_channels(new QUANTUM(nbChannels)), 
          m_alpha(new QUANTUM(nbAlphaChannels)), 
          m_colorSpace(colorSpace), 
          m_profile(profile) { };

        /**
         * Create a read/write pixel for existing channel data.
         */
        KisPixel(QUANTUM * channels, QUANTUM* alpha = 0, KisAbstractColorSpace * colorSpace = 0, KisProfileSP profile = 0)
                  : m_channels(channels),
                    m_alpha(alpha),
                    m_colorSpace(colorSpace),
                    m_profile(profile) {};


        virtual ~KisPixel() {}

public:

    KisQuantum operator[](int index) const { return KisQuantum(&m_channels[index]); };

    KisQuantum alpha() const { return KisQuantum(m_alpha); };

    KisAbstractColorSpace * colorSpace() const { return m_colorSpace; };

    void setProfile(KisProfileSP profile) { m_profile = profile; }

    KisProfileSP profile() const { return m_profile; }

    QUANTUM* channels() const { return m_channels; }
 

private:
    QUANTUM* m_channels;
    QUANTUM* m_alpha;
    KisAbstractColorSpace * m_colorSpace;
    KisProfileSP m_profile;
};

#endif
