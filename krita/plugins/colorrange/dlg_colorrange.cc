/*
 *  dlg_colorrange.cc - part of KimageShop^WKrayon^WKrita
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

#include "dlg_colorrange.h"
#include "wdg_colorrange.h"


DlgColorRange::DlgColorRange( QWidget *  parent,
			      const char * name)
	: super (parent, name, true, i18n("Color Range"), Ok | Cancel, Ok)
{
	m_page = new WdgColorRange(this, "color_range");
	setCaption(i18n("Color Range"));
	setMainWidget(m_page);
	resize(m_page -> sizeHint());

	connect(this, SIGNAL(okClicked()),
		this, SLOT(okClicked()));

	connect(m_page -> bnPickerPlus, SIGNAL(clicked()),
		this, SLOT(slotPickerPlusClicked()));

	connect(m_page -> bnPicker, SIGNAL(clicked()),
		this, SLOT(slotPickerClicked()));

	connect(m_page -> bnPickerMinus, SIGNAL(clicked()),
		this, SLOT(slotPickerMinusClicked()));

	connect(m_page -> bnLoad, SIGNAL(clicked()),
		this, SLOT(slotLoad()));

	connect(m_page -> bnSaveColorRange, SIGNAL(clicked()),
		this, SLOT(slotSave()));

	connect(m_page -> chkInvert, SIGNAL(clicked()),
		this, SLOT(slotInvertClicked()));

	connect(m_page -> intFuzziness, SIGNAL(valueChanged(int)),
		this, SLOT(slotFuzzinessChanged(int)));

	connect(m_page -> sldrFuzziness, SIGNAL(sliderMoved(int)),
		this, SLOT(slotSliderMoved(int)));

	connect(m_page -> cmbSelect, SIGNAL(activated(int)),
		this, SLOT(slotSelectionTypeChanged(int)));

	connect(m_page -> cmbSelectionPreview, SIGNAL(activated(int)),
		this, SLOT(slotPreviewTypeChanged(int)));

	m_page -> bnLoad -> setEnabled(false);
	m_page -> bnSaveColorRange -> setEnabled(false);
	m_page -> cmbSelectionPreview -> setEnabled(false);
}

DlgColorRange::~DlgColorRange()
{
	delete m_page;
}

void DlgColorRange::setLayer(KisLayerSP layer) 
{
	m_layer = layer;
}

void DlgColorRange::setSelection(KisSelectionSP selection)
{
	m_selection = selection;
	int w, h;
	w = m_page -> pixSelection -> width();
	h = m_page -> pixSelection -> height();
	
	// XXX: hardcoded size
	QPixmap pix = QPixmap(m_selection -> maskImage().scale(400, 350, QImage::ScaleMin));

	m_page -> pixSelection -> setPixmap(pix);
}

void DlgColorRange::okClicked()
{
	accept();
}


void DlgColorRange::slotPickerPlusClicked() 
{
}

void DlgColorRange::slotPickerClicked() 
{
}

void DlgColorRange::slotLoad() 
{
}

void DlgColorRange::slotPickerMinusClicked() 
{
}

void DlgColorRange::slotSave() 
{
}

void DlgColorRange::slotInvertClicked() 
{
}

void DlgColorRange::slotFuzzinessChanged(int value) 
{
	m_page -> sldrFuzziness -> setValue(value);
}

void DlgColorRange::slotSliderMoved(int value)
{
	m_page -> intFuzziness -> setValue(value);
}

void DlgColorRange::slotSelectionTypeChanged(int index) 
{
}

void DlgColorRange::slotPreviewTypeChanged(int index) 
{
}


#include "dlg_colorrange.moc"
