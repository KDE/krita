/*
 *  Copyright (c) 2004 Boudewijn Rempt
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

#include <kdebug.h>

#include "kis_histogram.h"
#include "kis_layer.h"
#include "kis_types.h"
#include "kis_iterators_pixel.h"

KisHistogram::KisHistogram(KisLayerSP layer,
			   const KisChannelInfo & initialChannel,
			   const enumHistogramType type)
{
	m_layer = layer;
	m_type = type;

	// XXX: size needs to come from KisChannelInfo, is determined by
	// the channel depth.
	Q_INT32 bincount = QUANTUM_MAX + 1;
 	m_values = vBins(bincount, 0);
	m_max = 0;
	m_min = QUANTUM_MAX;
	m_mean = QUANTUM_MAX / 2;
	m_median = QUANTUM_MAX / 2;
	m_stddev = QUANTUM_MAX / 2;
	m_pixels = 0;
	m_count = 0;
	m_high = 0;
	m_low = QUANTUM_MAX;
	m_percentile = 100;
	m_pixels = 1; // AUTOLAYER: should use layer extends.

	computeHistogramFor(initialChannel);
}

KisHistogram::~KisHistogram()
{
}

void KisHistogram::computeHistogramFor(const KisChannelInfo & channel)
{
	Q_UINT32 total = 0;
	Q_INT32 bincount = QUANTUM_MAX + 1;
	m_values = vBins(bincount, 0);
	m_count = 0;
	m_high = 0;
	m_low = QUANTUM_MAX;
	//Q_UINT32 total_white = 0;
	//Q_UINT32 total_black = 0;

		Q_INT32 x,y,w,h;
		m_layer->exactBounds(x,y,w,h);
		KisRectIteratorPixel srcIt = m_layer->createRectIterator(x,y,w,h, false);

		Q_INT32 channels = m_layer -> nChannels();
		bool alpha = m_layer -> alpha();
		while( ! srcIt.isDone() )
		{
			if (  !srcIt.isSelected()
				|| (alpha && ((QUANTUM)srcIt[channels - 1] == OPACITY_TRANSPARENT)) ) {
				++srcIt;
				continue;
			}
			QUANTUM datum = (QUANTUM)srcIt[channel.pos()];
			m_values[datum]++;
			if (datum > m_max) m_max = datum;
			if (datum < m_min) m_min = datum;
			if (m_values[datum] > m_high)
				m_high = m_values[datum];
			if (m_values[datum] < m_low)
				m_low = m_values[datum];
			total += datum;
			m_count++;
			++srcIt;
		}
	if (m_count > 0)
		m_mean = total / m_count;
	else
		m_mean = 0;
#if 0
	dump();
#endif
}


void KisHistogram::dump() {
	kdDebug(DBG_AREA_MATH) << "Histogram\n";

	switch (m_type) {
	case LINEAR:
		kdDebug(DBG_AREA_MATH) << "Linear histogram\n";
		break;
	case LOGARITHMIC:
		kdDebug(DBG_AREA_MATH) << "Logarithmic histogram\n";
	}

	kdDebug(DBG_AREA_MATH) << "Bins:\n";
        vBins::iterator it;
	QUANTUM i = 0;
        for( it = m_values.begin(); it != m_values.end(); ++it ) {
		kdDebug(DBG_AREA_MATH) << "Value "
			  << QString().setNum(i)
			  << ": "
			  <<  QString().setNum((*it))
			  << "\n";
		i++;
	}
	kdDebug(DBG_AREA_MATH) << "\n";

	kdDebug(DBG_AREA_MATH) << "Max: " << QString().setNum(m_max) << "\n";
	kdDebug(DBG_AREA_MATH) << "Min: " << QString().setNum(m_min) << "\n";
	kdDebug(DBG_AREA_MATH) << "High: " << QString().setNum(m_high) << "\n";
	kdDebug(DBG_AREA_MATH) << "Low: " << QString().setNum(m_low) << "\n";
	kdDebug(DBG_AREA_MATH) << "Mean: " << QString().setNum(m_mean) << "\n";
	kdDebug(DBG_AREA_MATH) << "Median: " << QString().setNum(m_median) << "\n";
	kdDebug(DBG_AREA_MATH) << "Stddev: " << QString().setNum(m_stddev) << "\n";
	kdDebug(DBG_AREA_MATH) << "pixels: " << QString().setNum(m_pixels) << "\n";
	kdDebug(DBG_AREA_MATH) << "count: " << QString().setNum(m_count) << "\n";
	kdDebug(DBG_AREA_MATH) << "percentile: " << QString().setNum(m_percentile) << "\n";

	kdDebug(DBG_AREA_MATH) << "\n";

}
