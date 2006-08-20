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

#include <kis_custom_image_widget.h>
#include <kis_doc.h>
#include <kis_meta_registry.h>
#include "kis_colorspace_factory_registry.h"
#include "kis_profile.h"
#include "kis_colorspace.h"
#include "kis_id.h"
#include "kis_cmb_idlist.h"
#include "squeezedcombobox.h"
#include "kis_color.h"
#include "kis_image.h"
#include "kis_layer.h"

#include <kcolorcombo.h>

#include <kdebug.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qtextedit.h>
#include <KoUnitWidgets.h>

KisCustomImageWidget::KisCustomImageWidget(QWidget *parent, KisDoc *doc, Q_INT32 defWidth, Q_INT32 defHeight, double resolution, QString defColorSpaceName, QString imageName)
    : WdgNewImage(parent) {
    m_doc = doc;

    txtName->setText(imageName);

    intWidth->setValue(defWidth);
    intHeight->setValue(defHeight);
    doubleResolution->setValue(resolution);

    cmbColorSpaces->setIDList(KisMetaRegistry::instance()->csRegistry()->listKeys());
    cmbColorSpaces->setCurrentText(defColorSpaceName);

    connect(cmbColorSpaces, SIGNAL(activated(const KisID &)),
        this, SLOT(fillCmbProfiles(const KisID &)));
    connect (m_createButton, SIGNAL( clicked() ), this, SLOT (buttonClicked()) );
    m_createButton -> setDefault(true);

    fillCmbProfiles(cmbColorSpaces->currentItem());

}

void KisCustomImageWidget::buttonClicked() {
    KisColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->getColorSpace(cmbColorSpaces->currentItem(), cmbProfile->currentText());

    QColor qc(cmbColor->color());

    m_doc->newImage(txtName->text(), (Q_INT32)intWidth->value(), (Q_INT32)intHeight->value(), cs, KisColor(qc, cs), txtDescription->text(), doubleResolution->value());
    KisImageSP img = m_doc->currentImage();
    if (img) {
        KisLayerSP layer = img->activeLayer();
        if (layer) {
            layer->setOpacity(backgroundOpacity());
        }
    }
    emit documentSelected();
}

Q_UINT8 KisCustomImageWidget::backgroundOpacity() const
{
    Q_INT32 opacity = sliderOpacity->value();

    if (!opacity)
        return 0;

    return (opacity * 255) / 100;
}

void KisCustomImageWidget::fillCmbProfiles(const KisID & s)
{
    cmbProfile->clear();

    if (!KisMetaRegistry::instance()->csRegistry()->exists(s)) {
        return;
    }

    KisColorSpaceFactory * csf = KisMetaRegistry::instance()->csRegistry()->get(s);
    if (csf == 0) return;

    QValueVector<KisProfile *>  profileList = KisMetaRegistry::instance()->csRegistry()->profilesFor( csf );
        QValueVector<KisProfile *> ::iterator it;
        for ( it = profileList.begin(); it != profileList.end(); ++it ) {
            cmbProfile->insertItem((*it)->productName());
    }
    cmbProfile->setCurrentText(csf->defaultProfile());
}

#include "kis_custom_image_widget.moc"
