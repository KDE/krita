/*
 *  Copyright (c) 2000 Michael Koch <koch@kde.org>
 *  Copyright (c) 2000 Patrick Julien <freak@codepimps.org>
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
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <qvbox.h>
#include <klineedit.h>
#include <klocale.h>
#include "kis_dlg_new_layer.h"

NewLayerDialog::NewLayerDialog(Q_INT32 maxWidth, Q_INT32 defWidth, Q_INT32 maxHeight, Q_INT32 defHeight, QWidget *parent, const char *name) 
	: super(parent, name, true, "", Ok | Cancel)
{
	QWidget *page = new QWidget(this);
	QGridLayout *grid;
	QLabel *lbl;

	setMainWidget(page);
	grid = new QGridLayout(page, 2, 2);
	setCaption(i18n("New Layer"));
	m_width = new QSpinBox( 1, maxWidth, 10, page);
	m_width -> setValue(defWidth);
	lbl = new QLabel(m_width, i18n("W&idth:"), page);

	grid -> addWidget(lbl, 0, 0);
	grid -> addWidget(m_width, 0, 1);

	m_height = new QSpinBox(1, maxHeight, 10, page);
	m_height -> setValue(defHeight);
	lbl = new QLabel(m_height, i18n("&Height:"), page);

	grid -> addWidget(lbl, 1, 0);
	grid -> addWidget(m_height, 1, 1);
}

#include "kis_dlg_new_layer.moc"

