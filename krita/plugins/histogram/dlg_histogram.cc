/*
 *  dlg_histogram.cc - part of KimageShop^WKrayon^WKrita
 *
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
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qslider.h>
#include <qcombobox.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qlabel.h>

#include <knuminput.h>
#include <klocale.h>

#include "kis_types.h"

#include "kis_layer.h"
#include "kis_selection.h"

#include "dlg_histogram.h"
#include "kis_histogram_widget.h"


DlgHistogram::DlgHistogram( QWidget *  parent,
			      const char * name)
	: super (parent, name, true, i18n("Histogram"), Ok | Cancel, Ok)
{
	m_page = new KisHistogramWidget(this, "histogram");
	setCaption(i18n("Histogram"));
	setMainWidget(m_page);
	resize(m_page -> sizeHint());
}

DlgHistogram::~DlgHistogram()
{
	delete m_page;
}

void DlgHistogram::setLayer(KisLayerSP layer) 
{
	m_layer = layer;
}

void DlgHistogram::setSelection(KisSelectionSP selection)
{
	m_selection = selection;	
}

void DlgHistogram::okClicked()
{
	accept();
}


#include "dlg_histogram.moc"
