/*
 *  dlg_rotateimage.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include <config.h>

#include <math.h>

#include <iostream>

using namespace std;

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include "dlg_oilpaintfilter.h"
#include "wdg_oilpaintfilter.h"


DlgOilPaintFilter::DlgOilPaintFilter( QWidget *  parent,
			    const char * name)
	: super (parent, name, true, i18n("Oil Paint"), Ok | Cancel, Ok)
{
	m_lock = false;

	m_page = new WdgOilPaintFilter(this, "oilpaint");

	setMainWidget(m_page);
	resize(m_page -> sizeHint());

	connect(this, SIGNAL(okClicked()),
		this, SLOT(okClicked()));

}

DlgOilPaintFilter::~DlgOilPaintFilter()
{
	delete m_page;
}

void DlgOilPaintFilter::setBrushSize(Q_UINT32 brushSize) 
{
	m_page -> brushSizeSpinBox -> setValue(brushSize);
	m_oldBrushSize = brushSize;

}

Q_UINT32 DlgOilPaintFilter::brushSize()
{
	return (Q_UINT32)qRound(m_page -> brushSizeSpinBox -> value());
}

void DlgOilPaintFilter::setSmooth(Q_UINT32 smooth) 
{
	m_page -> smoothSpinBox -> setValue(smooth);
	m_oldSmooth = smooth;

}

Q_UINT32 DlgOilPaintFilter::smooth()
{
	return (Q_UINT32)qRound(m_page -> smoothSpinBox -> value());
}

// SLOTS

void DlgOilPaintFilter::okClicked()
{
	accept();
}

#include "dlg_oilpaintfilter.moc"
