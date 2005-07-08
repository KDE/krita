/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
#include <qcolor.h>

#include "kis_types.h"
#include "kis_pixel.h"
#include "kis_color.h"
#include "kis_profile.h"
#include "kis_strategy_colorspace.h"
#include "kis_colorspace_registry.h"

KisColor::KisColor()
{
	m_data = 0;
	m_colorStrategy = 0;
	m_profile = 0;
}

KisColor::~KisColor()
{
	delete [] m_data;
}

KisColor::KisColor(const QColor & color)
{
	Q_ASSERT(color.isValid());
	
	m_colorStrategy = KisColorSpaceRegistry::instance()->get( KisID("RGBA", ""));
	m_data = new Q_UINT8[m_colorStrategy->pixelSize()];
	memset(m_data, 0, m_colorStrategy->pixelSize());
	m_colorStrategy->nativeColor(color, OPACITY_OPAQUE, m_data);
	m_profile = 0;
}

KisColor::KisColor(const QColor & color, KisStrategyColorSpaceSP colorStrategy, KisProfileSP profile)
	: m_colorStrategy(colorStrategy),
	  m_profile(profile)
{
	Q_ASSERT(color.isValid());
	
	m_data = new Q_UINT8[colorStrategy->pixelSize()];
	memset(m_data, 0, m_colorStrategy->pixelSize());
	m_colorStrategy->nativeColor(color, OPACITY_OPAQUE, m_data, profile);
}


KisColor::KisColor(const QColor & color, Q_UINT8 alpha, KisStrategyColorSpaceSP colorStrategy, KisProfileSP profile)
	: m_colorStrategy(colorStrategy),
	  m_profile(profile)
{
	Q_ASSERT(color.isValid());
	
	m_data = new Q_UINT8[colorStrategy->pixelSize()];
	memset(m_data, 0, m_colorStrategy->pixelSize());
	
	m_colorStrategy->nativeColor(color, alpha, m_data, profile);
}

KisColor::KisColor(const Q_UINT8 * data, KisStrategyColorSpaceSP colorStrategy, KisProfileSP profile)
	: m_colorStrategy(colorStrategy),
	  m_profile(profile)
{

	m_data = new Q_UINT8[colorStrategy->pixelSize()];
	memset(m_data, 0, m_colorStrategy->pixelSize());
	memmove(m_data, data, colorStrategy->pixelSize());
}


KisColor::KisColor(const KisColor &src, KisStrategyColorSpaceSP colorStrategy, KisProfileSP profile)
	: m_colorStrategy(colorStrategy),
	  m_profile(profile)
{
	m_data = new Q_UINT8[colorStrategy->pixelSize()];
	memset(m_data, 0, m_colorStrategy->pixelSize());
	// XXX: We shouldn't use KisPixel as an intermediary.
	// XXX: the position of the alpha channel is wrong, of course, but that doesn't hurt for the
	//      conversion and it's too costly to determine at the moment.
	KisPixel srcPixel = KisPixel(src.data(), src.data(), src.colorStrategy(), src.profile());
	KisPixel dstPixel = KisPixel(m_data, m_data, colorStrategy, profile);
	src.colorStrategy()->convertTo(srcPixel, dstPixel);
	
}

KisColor::KisColor(const KisColor & rhs)
{
	if (this == &rhs) return;
	
	m_colorStrategy = rhs.colorStrategy();
	m_data = new Q_UINT8[m_colorStrategy->pixelSize()];
	memset(m_data, 0, m_colorStrategy->pixelSize());
	memcpy(m_data, rhs.data(), m_colorStrategy->pixelSize());
	m_profile = rhs.profile();
	
}

KisColor & KisColor::operator=(const KisColor & rhs)
{
	delete [] m_data;
	m_data = 0;
	m_colorStrategy = rhs.colorStrategy();
	m_profile = rhs.profile();

	if (rhs.m_colorStrategy && rhs.m_data) {
		m_data = new Q_UINT8[m_colorStrategy->pixelSize()];
		memcpy(m_data, rhs.m_data, m_colorStrategy->pixelSize());
	}
	return * this;
}

void KisColor::convertTo(KisStrategyColorSpaceSP cs, KisProfileSP profile)
{
	kdDebug(DBG_AREA_CMS) << "Our colormodel: " << m_colorStrategy->id().name()
		  << ", new colormodel: " << cs->id().name() << "\n";
		  
	if (m_colorStrategy == cs && m_profile == profile) 
		return;

	Q_UINT8 * m_data2 = new Q_UINT8[cs->pixelSize()];
	memset(m_data2, 0, cs->pixelSize());

	m_colorStrategy->convertPixelsTo(m_data, m_profile, m_data2, cs, profile, 1);

	delete [] m_data;
	m_data = m_data2;
	m_colorStrategy = cs;
	m_profile = profile;

}


void KisColor::setColor(Q_UINT8 * data, KisStrategyColorSpaceSP colorStrategy, KisProfileSP profile)
{
	delete [] m_data;
	m_data = new Q_UINT8[colorStrategy->pixelSize()];
	memcpy(m_data, data, colorStrategy->pixelSize());
	m_colorStrategy = colorStrategy;
	m_profile = profile;
}

// To save the user the trouble of doing color->colorStrategy()->toQColor(color->data(), &c, &a, profile
void KisColor::toQColor(QColor *c) const
{
	if (m_colorStrategy && m_data) {
		// XXX (bsar): There must be a better way, but I'm getting hopelessly confused about constness by now
		KisStrategyColorSpaceSP cs(const_cast<KisStrategyColorSpace*>(m_colorStrategy.data()));
	
		cs->toQColor(m_data, c, m_profile);
	}
}

void KisColor::toQColor(QColor *c, QUANTUM *opacity) const
{
	if (m_colorStrategy && m_data) {
		// XXX (bsar): There must be a better way, but I'm getting hopelessly confused about constness by now
		KisStrategyColorSpaceSP cs(const_cast<KisStrategyColorSpace*>(m_colorStrategy.data()));
		cs->toQColor(m_data, c, opacity, m_profile);
	}
}

QColor KisColor::toQColor() const
{
	QColor c;
	toQColor(&c);
	return c;
}

void KisColor::dump() const
{
	
	kdDebug(DBG_AREA_CMS) << "KisColor (" << this << "), " << m_colorStrategy->id().name() << "\n";
	vKisChannelInfoSP channels = m_colorStrategy->channels();
		
	vKisChannelInfoSP_cit begin = channels.begin();
	vKisChannelInfoSP_cit end = channels.end();
	
	for (vKisChannelInfoSP_cit it = begin; it != end; ++it)
	{
		KisChannelInfoSP ch = (*it);
		// XXX: setNum always takes a byte.
		if (ch->size() == sizeof(Q_UINT8)) {
			// Byte
			kdDebug(DBG_AREA_CMS) << "Channel (byte): " << ch->name() << ": " << QString().setNum(m_data[ch->pos()]) << "\n";
		}
		else if (ch->size() == sizeof(Q_UINT16)) {
			// Short (may also by an nvidia half)
			kdDebug(DBG_AREA_CMS) << "Channel (short): " << ch->name() << ": " << QString().setNum(*((const Q_UINT16 *)(m_data+ch->pos())))  << "\n";
		}
		else if (ch->size() == sizeof(Q_UINT32)) {
			// Integer (may also be float... Find out how to distinguish these!)
			kdDebug(DBG_AREA_CMS) << "Channel (int): " << ch->name() << ": " << QString().setNum(*((const Q_UINT32 *)(m_data+ch->pos())))  << "\n";
		}
	}
	
}
