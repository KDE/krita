/*
 * This file is part of Krita
 *
 * Copyright (c) 1999 Matthias Elter (me@kde.org)
 * Copyright (c) 2001-2002 Igor Jansen (rm@kde.org)
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


#include <qlayout.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qtooltip.h>
#include <qcolor.h>

#include <kcolorbutton.h>
#include <klocale.h>

#include <koFrameButton.h>

#include <kis_canvas_subject.h>

#include "kis_wet_palette_widget.h"

KisWetPaletteWidget::KisWetPaletteWidget(QWidget *parent, const char *name) : super(parent, name)
{
	m_subject = 0;
	QGridLayout * l = new QGridLayout(this, 2, 8, 2, 2);
	l -> setAutoAdd(true);

	KColorButton * b;
	int WIDTH = 24;
	int HEIGHT = 24;

	b = new KColorButton(QColor(240, 32, 160), this);
	QToolTip::add(b, i18n("Quinacridone Rose"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(159, 88, 64), this);
	QToolTip::add(b,i18n("Indian Red"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(254, 220, 76), this);
	QToolTip::add(b,i18n("Cadmium Yellow"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(36, 180, 32), this);
	QToolTip::add(b,i18n("Hookers Green"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(16, 185, 215), this);
	QToolTip::add(b,i18n("Cerulean Blue"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(96, 32, 8), this);
	QToolTip::add(b,i18n("Burnt Umber"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(254, 96, 8), this);
	QToolTip::add(b,i18n("Cadmium Red"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(255, 136, 8), this);
	QToolTip::add(b,i18n("Brilliant Orange"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(240, 199, 8), this);
	QToolTip::add(b,i18n("Hansa Yellow"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(96, 170, 130), this);
	QToolTip::add(b,i18n("Phthalo Green"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(48, 32, 170), this);
	QToolTip::add(b,i18n("French Ultramarine"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(118, 16, 135), this);
	QToolTip::add(b,i18n("Interference Lilac"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(254, 254, 254), this);
	QToolTip::add(b,i18n("Titanium White"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(64, 64, 74), this);
	QToolTip::add(b,i18n("Ivory Black"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

	b = new KColorButton(QColor(255, 255, 255), this);
	QToolTip::add(b,i18n("Pure Water"));
	b -> setFixedSize(WIDTH, HEIGHT);
	connect(b, SIGNAL(changed(const QColor &)), SLOT(slotFGColorSelected(const QColor &)));

}

void KisWetPaletteWidget::update(KisCanvasSubject *subject)
{
	m_subject = subject;

}

void KisWetPaletteWidget::slotFGColorSelected(const QColor& c)
{
	if(m_subject)
		m_subject->setFGColor(c);
}

#include "kis_wet_palette_widget.moc"
