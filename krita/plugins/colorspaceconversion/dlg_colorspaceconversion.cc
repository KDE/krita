  /*
 *  dlg_colorspaceconversion.cc - part of KimageShop^WKrayon^WKrita
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

#include <config.h>

#include <math.h>

#include <iostream>

using namespace std;

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcombobox.h>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include "dlg_colorspaceconversion.h"
#include "wdgconvertcolorspace.h"

DlgColorspaceConversion::DlgColorspaceConversion( QWidget *  parent,
			    const char * name)
	: super (parent, name, true, i18n("Image Size"), Ok | Cancel, Ok)
{
	m_page = new WdgConvertColorSpace(this, "colorspace_conversion");

	setMainWidget(m_page);
	resize(m_page -> sizeHint());

	connect(this, SIGNAL(okClicked()),
		this, SLOT(okClicked()));

}

DlgColorspaceConversion::~DlgColorspaceConversion()
{
	delete m_page;
}

// SLOTS

void DlgColorspaceConversion::okClicked()
{
	accept();
}

#include "dlg_colorspaceconversion.moc"
