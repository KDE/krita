/*
 *  Copyright (c) 2002 Patrick Julien <freak@ideasandassociates.com>
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
#include <klocale.h>
#include <knuminput.h>
#include <kpushbutton.h>
#include "kis_dlg_paintoffset.h"
#include <qvbox.h>

KisDlgPaintOffset::KisDlgPaintOffset(Q_INT32 xoff, Q_INT32 yoff, QWidget *parent, const char *name) :
    super(parent, name, true, "", Ok | Cancel)
{
        QVBox *page = makeVBoxMainWidget();
	QGroupBox *grp = new QGroupBox(i18n("Pixel Paint Offset"), page);
	KPushButton *btn;
	QHBoxLayout *hbox;
	QVBoxLayout *vbox;
	QVBoxLayout *vboxchild;
	QLabel *lbl;
	KIntNumInput *ed;

	m_xoff = xoff;
	m_yoff = yoff;
	vbox = new QVBoxLayout(page);
	vbox -> insertSpacing(-1, 15);
	vbox -> addWidget(grp);
	vboxchild = new QVBoxLayout(grp);
	vboxchild -> insertSpacing(-1, 15);
	hbox = new QHBoxLayout(vboxchild);
	hbox -> insertSpacing(-1, 15);
	lbl = new QLabel(i18n("X Axis Offet:"), grp);
	hbox -> addWidget(lbl, 0, Qt::AlignLeft);
	ed = new KIntNumInput(xoff, grp);
	connect(ed, SIGNAL(valueChanged(int)), SLOT(xOffsetValue(int)));
	hbox -> addWidget(ed, 0, Qt::AlignLeft);
	hbox -> insertSpacing(-1, 15);
	hbox = new QHBoxLayout(vboxchild);
	hbox -> insertSpacing(-1, 15);
	lbl = new QLabel(i18n("Y Axis Offet:"), grp);
	hbox -> addWidget(lbl, 0, Qt::AlignLeft);
	ed = new KIntNumInput(yoff, grp);
	connect(ed, SIGNAL(valueChanged(int)), SLOT(yOffsetValue(int)));
	hbox -> addWidget(ed, 0, Qt::AlignLeft);
	hbox -> insertSpacing(-1, 15);
	vboxchild -> insertSpacing(-1, 15);
	vbox -> insertSpacing(-1, 15);
}

KisDlgPaintOffset::~KisDlgPaintOffset()
{
}

Q_INT32 KisDlgPaintOffset::xoff() const
{
	return m_xoff;
}

Q_INT32 KisDlgPaintOffset::yoff() const
{
	return m_yoff;
}

void KisDlgPaintOffset::xOffsetValue(int value)
{
	m_xoff = value;
}

void KisDlgPaintOffset::yOffsetValue(int value)
{
	m_yoff = value;
}

#include "kis_dlg_paintoffset.moc"
