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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <math.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <kdebug.h>

#include "kis_channelinfo.h"
#include "kis_histogram_widget.h"
#include "kis_histogram.h"
#include "kis_global.h"
#include "kis_types.h"

KisHistogramWidget::KisHistogramWidget(QWidget *parent, const char *name) 
	: super(parent, name)
{
}


KisHistogramWidget::~KisHistogramWidget()
{
}

void KisHistogramWidget::setChannels(vKisChannelInfoSP channels, Q_INT32 channelCount) 
{
	for (int i = 0; i < channelCount; i++) {
		KisChannelInfo* channel = channels[i];
		cmbChannel -> insertItem(channel -> name());
	}
}


void KisHistogramWidget::setHistogram(KisHistogramSP histogram) 
{
	Q_UINT32 height = pixHistogram -> height();
	m_histogram = histogram;
	// XXX should the width be resisable?
	m_pix = QPixmap(QUANTUM_MAX + 1, height);
	m_pix.fill();
  	QPainter p(&m_pix);
	p.setBrush(Qt::black);
	
	vBins::iterator it;
	Q_UINT32 i = 0;

	
	if (m_histogram -> getHistogramType() == LINEAR) {
		double factor = (double)height / (double)m_histogram -> getHighest();
		for( it = m_histogram -> begin(); it != m_histogram -> end(); ++it ) {
			p.drawLine(i, height, i, height - static_cast<Q_INT32>((double)(*it) * factor));
			i++;
		}
	} else {
		double factor = (double)height / (double)log(m_histogram -> getHighest());
		for( it = m_histogram -> begin(); it != m_histogram -> end(); ++it ) {
			p.drawLine(i, height, i, height - static_cast<Q_INT32>(log((double)*it) * factor));
			i++;
		}
	}


	pixHistogram -> setPixmap(m_pix);

}

#include "kis_histogram_widget.moc"

