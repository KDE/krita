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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <qlabel.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <klocale.h>
#include <knuminput.h>
#include "kis_dlg_dimension.h"

KisDlgDimension::KisDlgDimension(Q_INT32 maxWidth, 
		Q_INT32 defWidth, 
		Q_INT32 maxHeight, 
		Q_INT32 defHeight, 
		QWidget *parent, 
		const char *name) : super(parent, name, true, "", Ok | Cancel), m_width(defWidth), m_height(defHeight)
{
	QVBox *page = makeVBoxMainWidget();
	QGroupBox *grp = new QGroupBox(i18n("Dimensions"), page);
	QHBoxLayout *hbox;
	QVBoxLayout *vbox;
	QVBoxLayout *vboxchild;
	QLabel *lbl;
	QSpinBox *spin;

	vbox = new QVBoxLayout(page);
	vbox -> insertSpacing(-1, 15);
	vbox -> addWidget(grp);
	vboxchild = new QVBoxLayout(grp);
	vboxchild -> insertSpacing(-1, 15);
	hbox = new QHBoxLayout(vboxchild);
	hbox -> insertSpacing(-1, 15);
	lbl = new QLabel(i18n("Width:"), grp);
	hbox -> addWidget(lbl, 0, Qt::AlignLeft);
	spin = new KIntSpinBox(1, maxWidth, 10, defWidth, 10, grp);
	connect(spin, SIGNAL(valueChanged(int)), SLOT(widthChanged(int)));
	hbox -> addWidget(spin, 0, Qt::AlignLeft);
	hbox -> insertSpacing(-1, 15);
	hbox = new QHBoxLayout(vboxchild);
	hbox -> insertSpacing(-1, 15);
	lbl = new QLabel(i18n("Height:"), grp);
	hbox -> addWidget(lbl, 0, Qt::AlignLeft);
	spin = new KIntSpinBox(1, maxHeight, 10, defHeight, 10, grp);
	connect(spin, SIGNAL(valueChanged(int)), SLOT(heightChanged(int)));
	hbox -> addWidget(spin, 0, Qt::AlignLeft);
	hbox -> insertSpacing(-1, 15);
	vboxchild -> insertSpacing(-1, 15);
	vbox -> insertSpacing(-1, 15);
}

KisDlgDimension::~KisDlgDimension()
{
}

QSize KisDlgDimension::getSize() const
{
	return QSize(m_width, m_height);
}

void KisDlgDimension::widthChanged(int val)
{
	m_width = val;
}

void KisDlgDimension::heightChanged(int val)
{
	m_height = val;
}

#include "kis_dlg_dimension.moc"

