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

KisDlgCreateImg::KisDlgCreateImg(Q_INT32 maxWidth, Q_INT32 defWidth, Q_INT32 maxHeight, Q_INT32 defHeight, enumImgType defImgType, QWidget *parent, const char *name)
	: super(parent, name, true, "", Ok | Cancel), m_type(defImgType), m_opacity(OPACITY_OPAQUE)
{
	QWidget *page = new QWidget(this);
	QLabel *lbl;
	QVBoxLayout* layout;
	QGridLayout* grid;
	QButtonGroup *grp;
	QRadioButton *radio;

	setMainWidget(page);
	layout = new QVBoxLayout(page, 3);
	grid = new QGridLayout(layout, 2, 2);
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

	grp = new QButtonGroup(2, QGroupBox::Horizontal, i18n("Color Mode"), page);
	grp -> setExclusive(true);
	
	QRadioButton *indexedRadio = new QRadioButton(i18n("&Indexed"), grp);
	indexedRadio -> setEnabled(false);
	grp -> insert(indexedRadio, IMAGE_TYPE_INDEXEDA);

	QRadioButton *grayScaleRadio = new QRadioButton(i18n("&Grayscale"), grp);
	grayScaleRadio -> setEnabled(false);
	grp -> insert(grayScaleRadio, IMAGE_TYPE_GREYA);

	QRadioButton *rgbRadio = new QRadioButton(i18n("&RGB"), grp);
	rgbRadio -> setEnabled(true);
	grp -> insert(rgbRadio, IMAGE_TYPE_RGBA);

	QRadioButton *cmykRadio = new QRadioButton(i18n("&CMYK"), grp);
	cmykRadio -> setEnabled(true);
	grp -> insert(cmykRadio, IMAGE_TYPE_CMYKA);

	QRadioButton *labRadio = new QRadioButton(i18n("&LAB"), grp);
	labRadio -> setEnabled(false);
	grp -> insert(labRadio, IMAGE_TYPE_LABA);

	QRadioButton *yuvRadio = new QRadioButton(i18n("&YUV"), grp);
	yuvRadio -> setEnabled(false);
	grp -> insert(yuvRadio, IMAGE_TYPE_YUVA);

	switch (defImgType) {
	case IMAGE_TYPE_INDEXED:
	case IMAGE_TYPE_INDEXEDA:
		indexedRadio -> setChecked(true);
		break;
	case IMAGE_TYPE_GREY:
	case IMAGE_TYPE_GREYA:
		grayScaleRadio -> setChecked(true);
		break;
	default:
	case IMAGE_TYPE_RGB:
	case IMAGE_TYPE_RGBA:
		rgbRadio -> setChecked(true);
		break;
	case IMAGE_TYPE_CMYK:
	case IMAGE_TYPE_CMYKA:
		cmykRadio -> setChecked(true);
		break;
	case IMAGE_TYPE_YUV:
	case IMAGE_TYPE_YUVA:
		yuvRadio -> setChecked(true);
		break;
	case IMAGE_TYPE_LAB:
	case IMAGE_TYPE_LABA:
		labRadio -> setChecked(true);
		break;
	}

	connect(grp, SIGNAL(clicked(int)), SLOT(imgTypeClicked(int)));

	layout -> addWidget(grp);

	grp = new QButtonGroup(2, QGroupBox::Horizontal, i18n("Background"), page);
	grp -> setExclusive(true);
	radio = new QRadioButton(i18n("&Background color"), grp);
	radio = new QRadioButton(i18n("&Foreground color"), grp);
	radio = new QRadioButton(i18n("&White"), grp);
	radio -> setChecked(true);
	radio = new QRadioButton(i18n("&Transparent"), grp);
	layout -> addWidget(grp);
}

KisDlgCreateImg::~KisDlgCreateImg()
{
}

void KisDlgCreateImg::imgTypeClicked(int id)
{
	m_type = static_cast<enumImgType>(id);
}

#include "kis_dlg_create_img.moc"

