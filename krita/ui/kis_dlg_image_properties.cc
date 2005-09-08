/*
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
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qslider.h>
#include <qtextedit.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kcolorcombo.h>

#include <koUnitWidgets.h>

#include "kis_colorspace_registry.h"
#include "kis_dlg_image_properties.h"
#include "wdgnewimage.h"
#include "kis_profile.h"
#include "kis_types.h"
#include "kis_image.h"
#include "kis_config.h"
#include "kis_view.h"
#include "kis_id.h"
#include "kis_cmb_idlist.h"

KisDlgImageProperties::KisDlgImageProperties(KisImageSP image, QWidget *parent, const char *name)
    : super(parent, name, true, "", Ok | Cancel)
{

    setCaption(i18n("Image Properties"));
    m_page = new WdgNewImage(this);

    m_image = image;

    m_view = (KisView*)parent;

    setMainWidget(m_page);
    resize(m_page -> sizeHint());

    m_page -> txtName -> setText(image -> name());

    KisConfig cfg;

    m_page -> intWidth -> setValue(image -> width());
    m_page -> intWidth -> setMaxValue(cfg.maxImgWidth());
    m_page -> intHeight -> setValue(image -> height());
    m_page -> intHeight -> setMaxValue(cfg.maxImgHeight());

    m_page -> doubleResolution -> setValue(image -> xRes()); // XXX: separate values for x & y?

    m_page -> cmbColorSpaces -> hide();
    m_page -> lblColorSpaces -> setText(image->colorSpace()->id().name());


    fillCmbProfiles(image -> colorSpace() -> id());
    
    if (image -> profile()) {
        m_page -> cmbProfile -> setCurrentText(image -> profile() -> productName());
    }
    else {
        m_page -> cmbProfile -> setCurrentItem(0);
    }

    m_page -> sliderOpacity -> setEnabled(false); // XXX re-enable when figured out a way to do this
    m_page -> opacityPanel -> hide();
    m_page -> lblOpacity -> hide();
    
    m_page -> cmbColor -> setEnabled(false); // XXX re-enable when figured out a way to do this
    m_page -> cmbColor -> hide();
    m_page -> lblColor -> hide();

    connect(this, SIGNAL(okClicked()),
        this, SLOT(okClicked()));


    connect(m_page -> cmbColorSpaces, SIGNAL(activated(const KisID &)), 
        this, SLOT(fillCmbProfiles(const KisID &)));

    
}

KisDlgImageProperties::~KisDlgImageProperties()
{
    delete m_page;
}

void KisDlgImageProperties::okClicked()
{
    if (m_page -> intWidth -> value() != m_image -> width() ||
        m_page -> intHeight -> value() != m_image -> height()) {
        m_view -> resizeCurrentImage(m_page -> intWidth -> value(),
                             m_page -> intHeight -> value());
    }
    
    
    // XXX Convert m_page -> cmbColorSpaces -> currentText ();
    // XXX Convert background color
    // XXX: Convert opacity of background layer
    
    Q_INT32 opacity = m_page -> sliderOpacity -> value();
    opacity = opacity * 255 / 100;
    
    m_image -> setName(m_page -> txtName -> text());
    m_image -> setResolution(m_page -> doubleResolution -> value(), 
                 m_page -> doubleResolution -> value());
    m_image -> setDescription(m_page -> txtDescription -> text());

    vKisProfileSP profileList = m_image -> colorSpace() -> profiles();
    Q_UINT32 index = m_page -> cmbProfile -> currentItem();

    if (profileList.count() == 0 || 
        index > profileList.count() || 
        index == 0) {
        m_image -> setProfile(0);
    }
    else {
        m_image -> setProfile(profileList.at(index - 1));
    }
}

// XXX: Copy & paste from kis_dlg_create_img -- refactor to separate class
void KisDlgImageProperties::fillCmbProfiles(const KisID & s)
{

    KisAbstractColorSpace * cs = KisColorSpaceRegistry::instance() -> get(s);
    m_page -> cmbProfile -> clear();
    m_page -> cmbProfile -> insertItem(i18n("None"));
    vKisProfileSP profileList = cs -> profiles();
        vKisProfileSP::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
        m_page -> cmbProfile -> insertItem((*it) -> productName());
    }
    

}

#include "kis_dlg_image_properties.moc"

