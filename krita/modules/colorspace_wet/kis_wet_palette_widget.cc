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

#include "kis_wet_palette_widget.h"

#include <qlayout.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qspinbox.h>

#include <koFrameButton.h>
#include <koColorSlider.h>
#include <kcolordialog.h>
#include <kdualcolorbutton.h>

#include <qcolor.h>

KisWetPaletteWidget::KisWetPaletteWidget(QWidget *parent, const char *name) : super(parent, name)
{
	m_subject = 0;

}

void KisWetPaletteWidget::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	m_fgColor = subject->fgColor();
	m_bgColor = subject->bgColor();

}

void KisWetPaletteWidget::slotFGColorSelected(const QColor& c)
{
	m_fgColor = QColor(c);
	if(m_subject)
		m_subject->setFGColor(m_fgColor);
}

void KisWetPaletteWidget::slotBGColorSelected(const QColor& c)
{
	m_bgColor = QColor(c);
	if(m_subject)
		m_subject->setBGColor(m_bgColor);
}

#include "kis_wet_palette_widget.moc"
