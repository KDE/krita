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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "kis_profile.h"
#include "kis_colorspace.h"
#include <kis_id.h>

#include "dlg_colorspaceconversion.h"
#include "wdgconvertcolorspace.h"

DlgColorSpaceConversion::DlgColorSpaceConversion( QWidget *  parent,
                          const char * name)
    : super (parent, name, true, i18n("Image Size"), Ok | Cancel, Ok)
{
    m_page = new WdgConvertColorSpace(this, "colorspace_conversion");
    Q_CHECK_PTR(m_page);

    setMainWidget(m_page);
    resize(m_page -> sizeHint());

    m_page -> cmbColorSpaces -> setIDList(KisColorSpaceRegistry::instance() -> listKeys());

    fillCmbDestProfile(m_page -> cmbColorSpaces -> currentItem());

    // XXX: Until we have implemented high bit depth images
    m_page -> cmbDepth -> setEnabled(false);

    connect(m_page -> cmbColorSpaces, SIGNAL(activated(const KisID &)),
        this, SLOT(fillCmbDestProfile(const KisID &)));


    connect(this, SIGNAL(okClicked()),
        this, SLOT(okClicked()));

}

DlgColorSpaceConversion::~DlgColorSpaceConversion()
{
    delete m_page;
}

// SLOTS

void DlgColorSpaceConversion::okClicked()
{
    accept();
}


void DlgColorSpaceConversion::fillCmbDestProfile(const KisID & s)
{
    fillCmbProfile(m_page -> cmbDestProfile, s);

}

void DlgColorSpaceConversion::fillCmbSrcProfile(const KisID & s)
{
    fillCmbProfile(m_page -> cmbSourceProfile, s);

}

void DlgColorSpaceConversion::fillCmbProfile(QComboBox * cmb, const KisID& s)
{
    cmb -> clear();
    cmb -> insertItem(i18n("None"));

    KisColorSpace * cs = KisColorSpaceRegistry::instance() -> get(s);

    QValueVector<KisProfile *>  profileList = cs -> profiles();
        QValueVector<KisProfile *> ::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
        cmb -> insertItem((*it) -> productName());

    }
}


#include "dlg_colorspaceconversion.moc"
