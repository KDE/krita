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
	//Q_UINT32 total_white = 0;
	//Q_UINT32 total_black = 0;

	if (m_layer -> hasSelection()) {
		// Get selection iterators
		// XXX: not implemented yet
	}
	else {
		Q_INT32 x,y,w,h;
		m_layer->extent(x,y,w,h);
 		KisRectIteratorPixel srcIt = m_layer->createRectIterator(x,y,w,h, false);
		
		Q_INT32 depth = m_layer -> depth();
		while( ! srcIt.isDone() )
		{
			for( int i = 0; i < depth; i++)
			{
				// Do computing
				if (i == channel.pos()) {
					KisQuantum datum = srcIt[channel.pos()];
// 					m_values[datum] = m_values[datum]++;
// 					if (datum > m_max) m_max = datum;
// 						if (datum < m_min) m_min = datum;
// 					total += datum;
 					m_count++;
 				}
			}
			srcIt++;
		}
	}
	m_mean = total / m_count;
#if 0
	dump();
#endif
}


void KisHistogram::dump() {
	kdDebug() << "Histogram\n";

	switch (m_type) {
	case LINEAR:
		kdDebug() << "Linear histogram\n";
		break;
	case LOGARITHMIC:
		kdDebug() << "Logarithmic histogram\n";
	}

	kdDebug() << "Bins:\n";
        vBins::iterator it;
	QUANTUM i = 0;
        for( it = m_values.begin(); it != m_values.end(); ++it ) {
		kdDebug() << "Value " 
			  << QString().setNum(i)
			  << ": " 
			  <<  QString().setNum((*it)) 
			  << "\n";
		i++;
	}
	kdDebug() << "\n";

	kdDebug() << "Max: " << QString().setNum(m_max) << "\n";
	kdDebug() << "Min: " << QString().setNum(m_min) << "\n";
	kdDebug() << "Mean: " << QString().setNum(m_mean) << "\n";
	kdDebug() << "Median: " << QString().setNum(m_median) << "\n";
	kdDebug() << "Stddev: " << QString().setNum(m_stddev) << "\n";
	kdDebug() << "pixels: " << QString().setNum(m_pixels) << "\n";
	kdDebug() << "count: " << QString().setNum(m_count) << "\n";
	kdDebug() << "percentile: " << QString().setNum(m_percentile) << "\n";

	kdDebug() << "\n";

}
