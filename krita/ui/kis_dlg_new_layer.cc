/*
 *  Copyright (c) 2000 Michael Koch <koch@kde.org>
 *  Copyright (c) 2000 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Remot <boud@valdyas.org>
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
#include "integerwidget.h"

#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qvbox.h>

#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kpushbutton.h>

#include "kis_global.h"
#include "kis_cmb_composite.h"
#include "kis_cmb_imagetype.h"
#include "kis_dlg_new_layer.h"
#include "kis_dlg_paint_properties.h"
#include "kis_global.h"

NewLayerDialog::NewLayerDialog(Q_INT32 maxWidth, 
			       Q_INT32 defWidth, 
			       Q_INT32 maxHeight, 
			       Q_INT32 defHeight,
			       const QString colorSpaceName,
			       const QString & deviceName,
			       QWidget *parent, 
			       const char *name) 
	: super(parent, name, true, "", Ok | Cancel)
{
	QWidget *page = new QWidget(this);

	QGridLayout *grid;
	QLabel *lbl;

	setCaption(i18n("New Layer"));

	setMainWidget(page);
	grid = new QGridLayout(page, 8, 2);

	// Name
	lbl = new QLabel(i18n("Name:"), page);
	m_name = new KLineEdit(deviceName, page);
	grid -> addWidget(lbl, 0, 0);
	grid -> addWidget(m_name, 0, 1);
	
	// Opacity
	lbl = new QLabel(i18n("Opacity:"), page);
	m_opacity = new KIntNumInput(page);
	m_opacity -> setRange(0, 100, 13, true);
	m_opacity -> setValue(100);
	grid -> addWidget(lbl, 1, 0);
	grid -> addWidget(m_opacity, 1, 1);

	// Composite mode
	lbl = new QLabel(i18n("Composite mode:"), page);
	m_cmbComposite = new KisCmbComposite(page);
	m_cmbComposite -> setCurrentItem((int)COMPOSITE_OVER);
	grid -> addWidget(lbl, 2, 0);
	grid -> addWidget(m_cmbComposite, 2, 1);

	// Layer type
	lbl = new QLabel(i18n("Layer type:"), page);
	m_cmbImageType = new KisCmbImageType(page);
	m_cmbImageType -> setCurrentText(colorSpaceName);
	grid -> addWidget(lbl, 3, 0);
	grid -> addWidget(m_cmbImageType, 3, 1);

	// Width
	m_width = new QSpinBox( 1, maxWidth, 10, page);
	m_width -> setValue(defWidth);
	lbl = new QLabel(m_width, i18n("W&idth:"), page);
	grid -> addWidget(lbl, 4, 0);
	grid -> addWidget(m_width, 4, 1);

	// Height
	m_height = new QSpinBox(1, maxHeight, 10, page);
	m_height -> setValue(defHeight);
	lbl = new QLabel(m_height, i18n("&Height:"), page);
	grid -> addWidget(lbl, 5, 0);
	grid -> addWidget(m_height, 5, 1);

	// Position
	QGridLayout *gridInBox;
	QGroupBox *grp;

	grp = new QGroupBox(i18n("Position"), page);
	gridInBox = new QGridLayout(grp, 3, 2, 12);
	gridInBox -> setRowSpacing(0, 10);

	lbl = new QLabel(i18n("X axis:"), grp);
	m_x = new KIntSpinBox(SHRT_MIN, SHRT_MAX, 10, 0, 10, grp);
	gridInBox -> addWidget(lbl, 1, 0);
	gridInBox -> addWidget(m_x, 1, 1);

	lbl = new QLabel(i18n("Y axis:"), grp);
	m_y = new KIntSpinBox(SHRT_MIN, SHRT_MAX, 10, 0, 10, grp);
	gridInBox -> addWidget(lbl, 2, 0);
	gridInBox -> addWidget(m_y, 2, 1);

	grid -> addMultiCellWidget(grp, 6, 7, 0, 1);

}

int NewLayerDialog::opacity() const
{
	Q_INT32 opacity = m_opacity -> value();

	if (!opacity)
		return 0;

	opacity = opacity * 255 / 100;
	return upscale(opacity - 1);
}


QPoint NewLayerDialog::position() const
{
	return QPoint(m_x -> value(), m_y -> value());
}


CompositeOp NewLayerDialog::compositeOp() const
{
	return (CompositeOp)m_cmbComposite -> currentItem();
}

QString NewLayerDialog::colorStrategyName() const
{
	return m_cmbImageType -> currentText ();
}

QString NewLayerDialog::layerName() const
{
	return m_name -> text();
}


#include "kis_dlg_new_layer.moc"

