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

#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QList>

#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include "kis_meta_registry.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_profile.h"
#include "kis_colorspace.h"
#include "kis_id.h"
#include "kis_cmb_idlist.h"
#include "squeezedcombobox.h"

#include "dlg_colorspaceconversion.h"

DlgColorSpaceConversion::DlgColorSpaceConversion( QWidget *  parent,
                          const char * name)
    : super (parent, i18n("Image Size"), Ok | Cancel)
{
    setObjectName(name);
    m_page = new WdgConvertColorSpace(this);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("colorspace_conversion");

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_page->cmbColorSpaces->setIDList(KisMetaRegistry::instance()->csRegistry()->listKeys());

    fillCmbDestProfile(m_page->cmbColorSpaces->currentItem());

    m_intentButtonGroup.addButton(m_page->radioAbsoluteColorimetric, INTENT_ABSOLUTE_COLORIMETRIC);
    m_intentButtonGroup.addButton(m_page->radioPerceptual, INTENT_PERCEPTUAL);
    m_intentButtonGroup.addButton(m_page->radioRelativeColorimetric, INTENT_RELATIVE_COLORIMETRIC);
    m_intentButtonGroup.addButton(m_page->radioSaturation, INTENT_SATURATION);

    connect(m_page->cmbColorSpaces, SIGNAL(activated(const KisID &)),
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
    m_page->cmbDestProfile->clear();

    QList<KisProfile *>  profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor(s);

    foreach (KisProfile *profile, profileList) {
        m_page->cmbDestProfile->addSqueezedItem(profile->productName());

    }
}


#include "dlg_colorspaceconversion.moc"
