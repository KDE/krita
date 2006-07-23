/* This file is part of the KOffice project
 * Copyright (C) 2005 Thomas Zander <zander@kde.org>
 * Copyright (C) 2005 Casper Boemann <cbr@boemann.dk>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; version 2.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <QPushButton>
#include <QSlider>

#include <kcolorcombo.h>
#include <kdebug.h>

#include "KoUnitWidgets.h"

#include "kis_custom_image_widget.h"
#include "kis_doc.h"
#include "kis_meta_registry.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "KoID.h"
#include "kis_cmb_idlist.h"
#include "squeezedcombobox.h"
#include "KoColor.h"

KisCustomImageWidget::KisCustomImageWidget(QWidget *parent, KisDoc *doc, qint32 defWidth, qint32 defHeight, double resolution, QString defColorSpaceName, QString imageName)
    : WdgNewImage(parent) {
    m_doc = doc;

    txtName->setText(imageName);

    intWidth->setValue(defWidth);
    intHeight->setValue(defHeight);
    doubleResolution->setValue(resolution);

    cmbColorSpaces->setIDList(KisMetaRegistry::instance()->csRegistry()->listKeys());
    cmbColorSpaces->setCurrent(defColorSpaceName);

    connect(cmbColorSpaces, SIGNAL(activated(const KoID &)),
        this, SLOT(fillCmbProfiles(const KoID &)));
    connect (m_createButton, SIGNAL( clicked() ), this, SLOT (buttonClicked()) );

    fillCmbProfiles(cmbColorSpaces->currentItem());

}

void KisCustomImageWidget::buttonClicked() {
    KoColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->colorSpace(cmbColorSpaces->currentItem(), cmbProfile->currentText());

    QColor qc(cmbColor->color());

    m_doc->newImage(txtName->text(), (qint32)intWidth->value(), (qint32)intHeight->value(), cs, KoColor(qc, cs), txtDescription->toPlainText(), doubleResolution->value());
    emit documentSelected();
}

quint8 KisCustomImageWidget::backgroundOpacity() const
{
    qint32 opacity = sliderOpacity->value();

    if (!opacity)
        return 0;

    return (opacity * 255) / 100;
}

void KisCustomImageWidget::fillCmbProfiles(const KoID & s)
{
    cmbProfile->clear();

    if (!KisMetaRegistry::instance()->csRegistry()->exists(s)) {
        return;
    }

    KoColorSpaceFactory * csf = KisMetaRegistry::instance()->csRegistry()->get(s);
    if (csf == 0) return;

    QList<KoColorProfile *>  profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor( csf );

    foreach (KoColorProfile *profile, profileList) {
        cmbProfile->addSqueezedItem(profile->productName());
    }
    cmbProfile->setCurrent(csf->defaultProfile());
}

#include "kis_custom_image_widget.moc"
