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

#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qptrlist.h>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include <kis_factory.h>
#include <kis_colorspace_registry.h>
#include <kis_resourceserver.h>

#include "dlg_colorspaceconversion.h"
#include "wdgconvertcolorspace.h"

DlgColorspaceConversion::DlgColorspaceConversion( QWidget *  parent,
						  const char * name)
	: super (parent, name, true, i18n("Image Size"), Ok | Cancel, Ok)
{
	m_page = new WdgConvertColorSpace(this, "colorspace_conversion");

	setMainWidget(m_page);
	resize(m_page -> sizeHint());

	m_page -> cmbColorSpaces -> insertStringList(KisColorSpaceRegistry::instance() -> listColorSpaceNames());

	fillCmbDestProfile(m_page -> cmbColorSpaces -> currentText());

	// XXX: Until we have implemented high bit depth images
	m_page -> cmbDepth -> setEnabled(false);

	connect(m_page -> cmbColorSpaces, SIGNAL(activated(const QString &)), 
		this, SLOT(fillCmbDestProfile(const QString &)));


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


void DlgColorspaceConversion::fillCmbDestProfile(const QString & s) 
{
	fillCmbProfile(m_page -> cmbDestProfile, s);
	
}

void DlgColorspaceConversion::fillCmbSrcProfile(const QString & s)
{
	fillCmbProfile(m_page -> cmbSourceProfile, s);

}

void DlgColorspaceConversion::fillCmbProfile(QComboBox * cmb, const QString & s)
{
	cmb -> clear();

	KisStrategyColorSpaceSP cs = KisColorSpaceRegistry::instance() -> get(s);

	vKisProfileSP profileList = cs -> profiles();
        vKisProfileSP::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
		cmb -> insertItem((*it) -> productName());

	}
}


#include "dlg_colorspaceconversion.moc"
