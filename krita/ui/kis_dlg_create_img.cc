/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <klocale.h>
#include <koUnitWidgets.h>

#include "kis_dlg_create_img.h"

KisDlgCreateImg::KisDlgCreateImg(Q_INT32 maxWidth, Q_INT32 defWidth, Q_INT32 maxHeight, Q_INT32 defHeight, QString colorStrategyName, QWidget *parent, const char *name)
	: super(parent, name, true, "", Ok | Cancel), m_opacity(OPACITY_OPAQUE)
{

	// XXX: put this in UI file

	QWidget *page = new QWidget(this);
	QLabel *lbl;
	QVBoxLayout* layout;
	QGridLayout* grid;
	QRadioButton *radio;

	setMainWidget(page);
	layout = new QVBoxLayout(page, 3);
	grid = new QGridLayout(layout, 3, 2);
	setCaption(i18n("New Image"));
	m_widthSpin = new QSpinBox(1, maxWidth, 10, page);
	m_widthSpin -> setValue(defWidth);
	lbl = new QLabel(m_widthSpin, i18n("W&idth:"), page);

	grid -> addWidget(lbl, 0, 0);
	grid -> addWidget(m_widthSpin, 0, 1);

	m_heightSpin = new QSpinBox(1, maxHeight, 10, page);
	m_heightSpin -> setValue(defHeight);
	lbl = new QLabel(m_heightSpin, i18n("&Height:"), page);

	grid -> addWidget(lbl, 1, 0);
	grid -> addWidget(m_heightSpin, 1, 1);

	lbl = new QLabel(i18n("Layer type:"), page);
	m_cmbImageType = new KisCmbImageType(page);
	m_cmbImageType -> setCurrentText(colorStrategyName);
	grid -> addWidget(lbl, 2, 0);
	grid -> addWidget(m_cmbImageType, 2, 1);


	m_grp = new QButtonGroup(2, QGroupBox::Horizontal, i18n("Background"), page);
	m_grp -> setExclusive(true);
	radio = new QRadioButton(i18n("&Background color"), m_grp);
	radio = new QRadioButton(i18n("&Foreground color"), m_grp);
	radio = new QRadioButton(i18n("&White"), m_grp);
	radio -> setChecked(true);
	radio = new QRadioButton(i18n("&Transparent"), m_grp);
	layout -> addWidget(m_grp);
}

KisDlgCreateImg::~KisDlgCreateImg()
{
}


KoColor KisDlgCreateImg::backgroundColor() const
{
	switch(m_grp -> selectedId()) {
	case 0:
		return KoColor::white(); // Retrieve from kconfig
	case 1:
		return KoColor::black(); // Retrieve from kconfig
	case 2:
		return KoColor::white(); 
	case 3:
		return KoColor::white();
	}
	return KoColor::white();
}

QUANTUM KisDlgCreateImg::backgroundOpacity() const
{
	if (m_grp -> selectedId() == 3) {
		return OPACITY_TRANSPARENT;
	}
	else {
		return m_opacity;
	}
}

#include "kis_dlg_create_img.moc"

