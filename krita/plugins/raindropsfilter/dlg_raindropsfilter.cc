/*
 *  dlg_rotateimage.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include <config.h>

#include <math.h>

#include <iostream>

using namespace std;

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include "dlg_raindropsfilter.h"
#include "wdg_raindropsfilter.h"


DlgRainDropsFilter::DlgRainDropsFilter( QWidget *  parent,
			    const char * name)
	: super (parent, name, true, i18n("Raindrops"), Ok | Cancel, Ok)
{
	m_lock = false;

	m_page = new WdgRainDropsFilter(this, "raindrops");

	setMainWidget(m_page);
	resize(m_page -> sizeHint());

	connect(this, SIGNAL(okClicked()),
		this, SLOT(okClicked()));

}

DlgRainDropsFilter::~DlgRainDropsFilter()
{
	delete m_page;
}

void DlgRainDropsFilter::setDropSize(Q_UINT32 dropSize) 
{
	m_page -> dropSizeSpinBox -> setValue(dropSize);
	m_oldDropSize = dropSize;

}

Q_UINT32 DlgRainDropsFilter::dropSize()
{
	return (Q_UINT32)qRound(m_page -> dropSizeSpinBox -> value());
}

void DlgRainDropsFilter::setNumber(Q_UINT32 number) 
{
	m_page -> numberSpinBox -> setValue(number);
	m_oldNumber = number;

}

Q_UINT32 DlgRainDropsFilter::number()
{
	return (Q_UINT32)qRound(m_page -> numberSpinBox -> value());
}

void DlgRainDropsFilter::setFishEyes(Q_UINT32 fishEyes) 
{
	m_page -> fishEyesSpinBox -> setValue(fishEyes);
	m_oldFishEyes = fishEyes;

}

Q_UINT32 DlgRainDropsFilter::fishEyes()
{
	return (Q_UINT32)qRound(m_page -> fishEyesSpinBox -> value());
}

// SLOTS

void DlgRainDropsFilter::okClicked()
{
	accept();
}

#include "dlg_raindropsfilter.moc"
