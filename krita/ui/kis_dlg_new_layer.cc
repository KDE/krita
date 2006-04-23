/*
 *  Copyright (c) 2000 Michael Koch <koch@kde.org>
 *  Copyright (c) 2000 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Remot <boud@valdyas.org>
 *  Copyright (c) 2006 Casper Boemann <cbr@boemann.dk>
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
#include <QGroupBox>
#include <qlabel.h>
#include <qlayout.h>

#include <klineedit.h>
#include <klocale.h>
#include <knuminput.h>
#include <kpushbutton.h>

#include "kis_factory.h"
#include "kis_global.h"
#include "kis_cmb_composite.h"
#include "kis_cmb_idlist.h"
#include "squeezedcombobox.h"
#include "kis_dlg_new_layer.h"
#include "kis_meta_registry.h"
#include "kis_colorspace_factory_registry.h"
#include "kis_profile.h"
#include "kis_colorspace.h"
#include "kis_int_spinbox.h"
#include "kis_dlg_layer_properties.h"

NewLayerDialog::NewLayerDialog(const KisID colorSpaceID,
                   const QString & profilename,
                   const QString & deviceName,
                   QWidget *parent,
                   const char *name)
    : super(parent, i18n("New Layer"), Ok | Cancel)
{
    setObjectName(name);
    m_page = new WdgLayerProperties(this);
    m_page->layout()->setMargin(0);

    setMainWidget(m_page);

    // Name
    m_page->editName->setText(deviceName);

    // Opacity
    m_page->intOpacity->setRange(0, 100, 13);
    m_page->intOpacity->setValue(100);

    // ColorSpace
    m_page->cmbColorSpaces->setIDList(KisMetaRegistry::instance()->csRegistry()->listKeys());
    m_page->cmbColorSpaces->setCurrentText(colorSpaceID.id());
    connect(m_page->cmbColorSpaces, SIGNAL(activated(const KisID &)),
        this, SLOT(fillCmbProfiles(const KisID &)));
    connect(m_page->cmbColorSpaces, SIGNAL(activated(const KisID &)),
        this, SLOT(fillCmbComposite(const KisID &)));

    // Init profiles
    fillCmbProfiles(m_page->cmbColorSpaces->currentItem());
    m_page->cmbProfile->setCurrentIndexFromText(profilename);

    // Init composite op
    fillCmbComposite(m_page->cmbColorSpaces->currentItem());

/*
    connect( m_page->editName, SIGNAL( textChanged ( const QString & ) ), this, SLOT( slotNameChanged( const QString & ) ) );

    slotNameChanged( m_page->editName->text() );
*/
}

void NewLayerDialog::setColorSpaceEnabled(bool enabled)
{
    m_page->cmbProfile->setEnabled(enabled);
    m_page->cmbColorSpaces->setEnabled(enabled);
}

void NewLayerDialog::fillCmbProfiles(const KisID & s)
{
    m_page->cmbProfile->clear();

    if (!KisMetaRegistry::instance()->csRegistry()->exists(s)) {
        return;
    }

    KisColorSpaceFactory * csf = KisMetaRegistry::instance()->csRegistry()->get(s);
    if (csf == 0) return;

    Q3ValueVector<KisProfile *>  profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor( csf );
        Q3ValueVector<KisProfile *> ::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
            m_page->cmbProfile->addSqueezedItem((*it)->productName());
    }
    m_page->cmbProfile->setCurrentIndexFromText(csf->defaultProfile());
}

void NewLayerDialog::fillCmbComposite(const KisID & s)
{
    m_page->cmbComposite->clear();

    if (!KisMetaRegistry::instance()->csRegistry()->exists(s)) {
        return;
    }

    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getColorSpace(s,"");
    if (cs) {
        m_page->cmbComposite->setCompositeOpList(cs->userVisiblecompositeOps());
    }
}

int NewLayerDialog::opacity() const
{
    qint32 opacity = m_page->intOpacity->value();

    if (!opacity)
        return 0;

    opacity = int((opacity * 255.0) / 100 + 0.5);
    if(opacity>255)
        opacity=255;
    return opacity;
}

KisCompositeOp NewLayerDialog::compositeOp() const
{
    return m_page->cmbComposite->currentItem();
}

KisID NewLayerDialog::colorSpaceID() const
{
    return m_page->cmbColorSpaces->currentItem();
}

QString NewLayerDialog::layerName() const
{
    return m_page->editName->text();
}

QString NewLayerDialog::profileName() const
{
    return m_page->cmbProfile-> currentText();
}

#include "kis_dlg_new_layer.moc"

