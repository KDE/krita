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
#ifndef KIS_HISTOGRAM_WIDGET_
#define KIS_HISTOGRAM_WIDGET_

#include <qwidget.h>
#include <qpixmap.h>

#include "kis_types.h"

/**
 * The histogram widget takes a paint device or an image and
 * draws a histogram for the given KisHistogram.
 */
class KisHistogramWidget : public QWidget {

	typedef QWidget super;
	Q_OBJECT


public:
	KisHistogramWidget(QWidget *parent, const char *name);
	virtual ~KisHistogramWidget();

	void setHistogram(KisHistogramSP histogram);

private:

	QPixmap m_pix;
	KisHistogramSP m_histogram;
};


#endif // KIS_HISTOGRAM_WIDGET_
