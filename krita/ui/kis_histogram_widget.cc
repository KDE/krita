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

void KisHistogramWidget::setChannels(ChannelInfo * channels, Q_INT32 channelCount) 
{
	for (int i = 0; i < channelCount; i++) {
		ChannelInfo channel = channels[i];
		cmbChannel -> insertItem(channel.name());
	}
}


void KisHistogramWidget::setHistogram(KisHistogramSP histogram) 
{
	m_histogram = histogram;
	m_pix = QPixmap(QUANTUM_MAX + 1, m_histogram -> getMax());
	m_pix.fill();
  	QPainter p(&m_pix);
	p.setBrush(Qt::black);
	
	vBins::iterator it;
 	Q_UINT32 i = 0;
	for( it = m_histogram -> begin(); it != m_histogram -> end(); ++it ) {
		kdDebug() << "Value " 
			  << QString().setNum(i)
			  << ": " 
			  <<  QString().setNum((*it)) 
			  << "\n";
		
 		p.drawLine(i, 0, i, (*it));
 		i++;
 	}
	
 	pixHistogram -> setPixmap(m_pix);

}

#include "kis_histogram_widget.moc"

