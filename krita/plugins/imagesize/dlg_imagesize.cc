/*
 *  dlg_imagesize.cc - part of KimageShop^WKrayon^WKrita
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
#include <iostream>

using namespace std;

#include <qcombobox.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include "dlg_imagesize.h"
#include "wdg_imagesize.h"

namespace {

	enum enumScaleType {
		PERCENT,
		PIXEL
	};

}

// XXX: I'm really real bad at arithmetic, let alone math. Here
// be rounding errors. (Boudewijn)

DlgImageSize::DlgImageSize( QWidget *  parent,
			    const char * name)
	: super (parent, name, true, i18n("Image Size"), Ok | Cancel, Ok)
{
	m_lock = false;

	m_page = new WdgImageSize(this, "image_size");
	setCaption(i18n("Image Size"));
	setMainWidget(m_page);
	resize(m_page -> sizeHint());

	connectAll();


	connect(this, SIGNAL(okClicked()),
		this, SLOT(okClicked()));


	connect(m_page -> chkConstrain, SIGNAL(toggled(bool)),
		this, SLOT(slotConstrainToggled(bool)));


	// Still unimplemented
        m_page -> cmbPrintWidthUnit -> setEnabled(false);
        m_page -> cmbPrintHeightUnit -> setEnabled(false);
        m_page -> cmbXResolutionType -> setEnabled(false);
        m_page -> cmbYResolutionType -> setEnabled(false);
        m_page -> intPrintHeight -> setEnabled(false);
        m_page -> intPrintWidth -> setEnabled(false);
        m_page -> dblXRes -> setEnabled(false);
        m_page -> dblYRes -> setEnabled(false);
}

DlgImageSize::~DlgImageSize()
{
	delete m_page;
}

void DlgImageSize::setWidth(Q_UINT32 w) 
{
	disconnectAll();

	m_page -> intWidth -> setValue(w);
	m_oldW = w;
	m_origW = w;

	connectAll();
}

void DlgImageSize::setMaximumWidth(Q_UINT32 w)
{
	m_page -> intWidth -> setMaxValue(w);
	m_maxW = w;
}

Q_INT32 DlgImageSize::width()
{
	return (Q_INT32)round(m_oldW);
}

void DlgImageSize::setHeight(Q_UINT32 h)
{
	disconnectAll();
	
	m_page -> intHeight -> setValue(h);
	m_oldH = h;
	m_origH = h;

	connectAll();
}


void DlgImageSize::setMaximumHeight(Q_UINT32 h)
{
	m_page -> intHeight -> setMaxValue(h);
	m_maxH = h;
}


Q_INT32 DlgImageSize::height()
{
	return (Q_INT32)round(m_oldH);
}

void DlgImageSize::setXRes(double x)
{
	m_page -> dblXRes -> setValue(x);
}

void DlgImageSize::setYRes(double y) 
{
	m_page -> dblYRes -> setValue(y);
}

bool DlgImageSize::scale() 
{
	return m_page -> chkResample -> isChecked();
}

// SLOTS

void DlgImageSize::okClicked()
{
	accept();
}

void DlgImageSize::slotWidthChanged(int w)
{
	if (m_page -> chkConstrain -> isChecked()) {

		disconnectAll();

		if (m_page -> cmbScaleTypeW -> currentItem() == PERCENT) {
			m_oldH = (m_origH * w) / 100.0;

			if (m_oldH > m_maxH) m_oldH = m_maxH;

			if (m_page -> cmbScaleTypeH -> currentItem() == PERCENT) {
				m_page -> intHeight -> setValue(w);
			}
			else {
				m_page -> intHeight -> setValue((int)round(m_oldH));
			}
		}
		else {
			double percent = (w / m_origW);

			m_oldH = round(m_origH * percent);

			if (m_page -> cmbScaleTypeH -> currentItem() == PERCENT) {
				m_page -> intHeight -> setValue((int)round(percent));
			}
			else {
				m_page -> intHeight -> setValue((int)m_oldH);
			}
		}

		connectAll();
	}
	if (m_page -> cmbScaleTypeW -> currentItem() == PERCENT) {
		m_oldW = (m_origW * w) / 100;
	}
	else {
		m_oldW = w;
	}

}

void DlgImageSize::slotHeightChanged(int h)
{
	if (m_page -> chkConstrain -> isChecked()) {

		disconnectAll();

		if (m_page -> cmbScaleTypeH -> currentItem() == PERCENT) {
			m_oldW = (m_origW * h) / 100.0;
			if (m_oldW > m_maxW) m_oldW = m_maxW;

			if (m_page -> cmbScaleTypeW -> currentItem() == PERCENT) {
				m_page -> intWidth -> setValue(h);
			}
			else {
				m_page -> intWidth -> setValue((int)round(m_oldW));
			}
		}
		else {
			double percent = (h / m_origH);

			m_oldW = round(m_origH * percent);

			if (m_page -> cmbScaleTypeW -> currentItem() == PERCENT) {
				m_page -> intWidth -> setValue((int)round(percent));
			}
			else {
				m_page -> intWidth -> setValue((int)m_oldW);
			}
		}

		connectAll();
	}

	if (m_page -> cmbScaleTypeH -> currentItem() == PERCENT) {
		m_oldH = (m_origH * h) / 100;
	}
	else {
		m_oldH = h;
	}

}

void DlgImageSize::slotScaleTypeWChanged(int i)
{
	disconnectAll();

	if (i == PERCENT) {
		m_page -> intWidth -> setValue((int)round((m_oldW / m_origW) * 100));
	}
	else {
		m_page -> intWidth -> setValue((int)round(m_oldW));
	}

	connectAll();
}

void DlgImageSize::slotScaleTypeHChanged(int i)
{

	disconnectAll();

	if (i == PERCENT) {
		m_page -> intHeight -> setValue((int)round((m_oldH / m_origH) * 100));
	}
	else {
		m_page -> intHeight -> setValue((int)round(m_oldH));
	}

	connectAll();
}

void DlgImageSize::slotConstrainToggled(bool b) 
{
	m_origW = m_oldW;
	m_origH = m_oldH;
}

void DlgImageSize::disconnectAll()
{
	m_page -> intWidth -> disconnect();
	m_page -> intHeight -> disconnect();
	m_page -> cmbScaleTypeW -> disconnect();
	m_page -> cmbScaleTypeH -> disconnect();
}

void DlgImageSize::connectAll()
{

	connect (m_page -> intWidth, SIGNAL(valueChanged(int)),
		 this, SLOT(slotWidthChanged(int)));

	connect (m_page -> intHeight, SIGNAL(valueChanged(int)),
		 this, SLOT(slotHeightChanged(int)));

	connect (m_page -> cmbScaleTypeW, SIGNAL(activated(int)),
		 this, SLOT(slotScaleTypeWChanged(int)));


	connect (m_page -> cmbScaleTypeH, SIGNAL(activated(int)),
		 this, SLOT(slotScaleTypeHChanged(int)));

}

#include "dlg_imagesize.moc"
