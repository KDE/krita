/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include <qwidget.h>
#include <qcursor.h>

#include <klocale.h>
#include <kcolorcombo.h>
#include <kdebug.h>

#include <koUnitWidgets.h>

#include "kis_colorspace_factory_registry.h"
#include "kis_dlg_create_img.h"
#include "wdgnewimage.h"
#include "kis_profile.h"
#include "kis_colorspace.h"
#include "kis_id.h"
#include "kis_cmb_idlist.h"

KisDlgCreateImg::KisDlgCreateImg(Q_INT32 maxWidth, Q_INT32 defWidth, 
                 Q_INT32 maxHeight, Q_INT32 defHeight, 
                 QString colorSpaceName, QString imageName,
                 QWidget *parent, const char *name)
    : super(parent, name, true, "", Ok | Cancel)
{

    setCaption(i18n("New Image"));

    m_page = new WdgNewImage(this);

    setMainWidget(m_page);
    resize(m_page -> sizeHint());

    m_page -> txtName -> setText(imageName);

    m_page -> intWidth -> setValue(defWidth);
    m_page -> intWidth -> setMaxValue(maxWidth);
    m_page -> intHeight -> setValue(defHeight);
    m_page -> intHeight -> setMaxValue(maxHeight);
    m_page -> doubleResolution -> setValue(100.0); // XXX: Get this from settings?

    m_page -> cmbColorSpaces -> setIDList(KisColorSpaceFactoryRegistry::instance() -> listKeys());
    m_page -> cmbColorSpaces -> setCurrentText(colorSpaceName);

    connect(m_page -> cmbColorSpaces, SIGNAL(activated(const KisID &)), 
        this, SLOT(fillCmbProfiles(const KisID &)));

    // Temporary KisID; this will be matched to the translated ID in the current KisIDList.
    fillCmbProfiles(KisID(colorSpaceName, ""));

}

KisDlgCreateImg::~KisDlgCreateImg()
{
    delete m_page;
}

Q_INT32 KisDlgCreateImg::imgWidth() const
{
    return m_page -> intWidth -> value();
}

Q_INT32 KisDlgCreateImg::imgHeight() const
{
    return m_page -> intHeight -> value();
}

KisID KisDlgCreateImg::colorSpaceID() const
{
    return m_page -> cmbColorSpaces -> currentItem();
}

QColor KisDlgCreateImg::backgroundColor() const
{
    return QColor(m_page -> cmbColor -> color());
}

Q_UINT8 KisDlgCreateImg::backgroundOpacity() const
{
    // XXX: This widget is sizeof quantum dependent. Scale
    // to selected bit depth.
    Q_INT32 opacity = m_page -> sliderOpacity -> value();

    if (!opacity)
        return 0;

    opacity = opacity * 255 / 100;
    return opacity;
}

QString KisDlgCreateImg::imgName() const
{
    return m_page -> txtName -> text();
}

double KisDlgCreateImg::imgResolution() const
{
    return m_page -> doubleResolution -> value();
 }

QString KisDlgCreateImg::imgDescription() const
{
    return m_page -> txtDescription -> text();
}

QString KisDlgCreateImg::profileName() const
{
    return m_page -> cmbProfile -> currentText();
}

void KisDlgCreateImg::fillCmbProfiles(const KisID & s)
{


    m_page -> cmbProfile -> clear();

    KisColorSpaceFactory * csf = KisColorSpaceFactoryRegistry::instance() -> get(s);
    if (csf == 0) return;

    QValueVector<KisProfile *>  profileList = KisColorSpaceFactoryRegistry::instance()->profilesFor( csf );
        QValueVector<KisProfile *> ::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
            m_page -> cmbProfile -> insertItem((*it) -> productName());
    }
}

#include "kis_dlg_create_img.moc"

