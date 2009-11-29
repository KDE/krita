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

#include "kis_dlg_image_properties.h"

#include <QPushButton>
#include <QRadioButton>
#include <QLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QTextEdit>

#include <klocale.h>
#include <kcolorcombo.h>

#include <KoUnitDoubleSpinBox.h>
#include <KoColorSpace.h>
#include "KoColorSpaceRegistry.h"
#include "KoColorProfile.h"
#include "KoID.h"
#include "kis_types.h"
#include "kis_image.h"

#include "kis_config.h"
#include "kis_factory2.h"
#include "widgets/kis_cmb_idlist.h"
#include "widgets/squeezedcombobox.h"

KisDlgImageProperties::KisDlgImageProperties(KisImageWSP image, QWidget *parent, const char *name)
        : KDialog(parent)
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);
    setCaption(i18n("Image Properties"));
    m_page = new WdgImageProperties(this);

    m_image = image;

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    KisConfig cfg;

    m_page->lblWidthValue->setText(QString::number(image->width()));
    m_page->lblHeightValue->setText(QString::number(image->height()));

    m_page->lblResolutionValue->setText(KGlobal::locale()->formatNumber(image->xRes()*72, 2)); // XXX: separate values for x & y?

    QList<KoID> colorSpaces = KoColorSpaceRegistry::instance()->listKeys();
    qint32 i = colorSpaces.indexOf(KoID("WET", ""));
    if (i >= 0) {
        colorSpaces.removeAt(i);
    }
    m_page->cmbColorSpaces->setIDList(colorSpaces);
    m_page->cmbColorSpaces->setCurrent(image->colorSpace()->id());

    fillCmbProfiles(KoID(image->colorSpace()->id()));

    if (image->profile()) {
        m_page->cmbProfile->setCurrent(image->profile()->name());
    } else {
        m_page->cmbProfile->setCurrentIndex(0);
    }

    connect(m_page->cmbColorSpaces, SIGNAL(activated(const KoID &)),
            this, SLOT(fillCmbProfiles(const KoID &)));
}

KisDlgImageProperties::~KisDlgImageProperties()
{
    delete m_page;
}

const KoColorSpace * KisDlgImageProperties::colorSpace()
{
    return KoColorSpaceRegistry::instance()->colorSpace(m_page->cmbColorSpaces->currentItem(), m_page->cmbProfile->itemHighlighted());
}

const KoColorProfile * KisDlgImageProperties::profile()
{
    QList<const KoColorProfile *>  profileList = KoColorSpaceRegistry::instance()->profilesFor(m_image->colorSpace()->id());
    qint32 index = m_page->cmbProfile->currentIndex();

    if (index < profileList.count()) {
        return profileList.at(index);
    } else {
        return 0;
    }
}

// XXX: Copy & paste from kis_dlg_create_image -- refactor to separate class
void KisDlgImageProperties::fillCmbProfiles(const KoID & s)
{
    KoColorSpaceFactory * csf = KoColorSpaceRegistry::instance()->value(s.id());
    m_page->cmbProfile->clear();
    if (csf) {
        QList<const KoColorProfile *>  profileList = KoColorSpaceRegistry::instance()->profilesFor(csf);

        foreach(const KoColorProfile *profile, profileList) {
            m_page->cmbProfile->addSqueezedItem(profile->name());
        }
    }
}

#include "kis_dlg_image_properties.moc"

