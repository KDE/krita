/* This file is part of the KOffice project
 * Copyright (C) 2005 Thomas Zander <zander@kde.org>
 * Copyright (C) 2005 Casper Boemann <cbr@boemann.dk>
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
#include "kis_custom_image_widget.h"

#include <QPushButton>
#include <QSlider>
#include <QComboBox>

#include <kcolorcombo.h>
#include <kdebug.h>

#include "KoUnitWidgets.h"
#include "KoColorSpaceRegistry.h"
#include "KoColorProfile.h"
#include "KoColorSpace.h"
#include "KoID.h"
#include "KoColor.h"
#include <KoUnit.h>

#include "kis_doc2.h"
#include "kis_meta_registry.h"
#include "kis_cmb_idlist.h"
#include "squeezedcombobox.h"
#include "kis_image.h"
#include "kis_layer.h"

KisCustomImageWidget::KisCustomImageWidget(QWidget *parent, KisDoc2 *doc, qint32 defWidth, qint32 defHeight, double resolution, const QString & defColorSpaceName, const QString & imageName)
    : WdgNewImage(parent) {
    m_doc = doc;

    txtName->setText(imageName);

    m_widthUnit = KoUnit(KoUnit::Pixel, resolution);
    doubleWidth->setValue(defWidth);
    doubleWidth->setDecimals(0);
    m_width = KoUnit::fromUserValue(defWidth, m_widthUnit);
    cmbWidthUnit->addItems( KoUnit::listOfUnitName(false) );
    cmbWidthUnit->setCurrentIndex(KoUnit::Pixel);

    m_heightUnit = KoUnit(KoUnit::Pixel, resolution);
    doubleHeight->setValue(defHeight);
    doubleHeight->setDecimals(0);
    m_height = KoUnit::fromUserValue(defHeight, m_heightUnit);
    cmbHeightUnit->addItems( KoUnit::listOfUnitName(false) );
    cmbHeightUnit->setCurrentIndex(KoUnit::Pixel);

    doubleResolution->setValue(72.0 * resolution);
    doubleResolution->setDecimals(0);

    cmbColorSpaces->setIDList(KisMetaRegistry::instance()->csRegistry()->listKeys());
    cmbColorSpaces->setCurrent(defColorSpaceName);

    connect(cmbWidthUnit, SIGNAL(activated(int)),
        this, SLOT(widthUnitChanged(int)));
    connect(doubleWidth, SIGNAL(valueChanged(double)),
        this, SLOT(widthChanged(double)));
    connect(cmbHeightUnit, SIGNAL(activated(int)),
        this, SLOT(heightUnitChanged(int)));
    connect(doubleHeight, SIGNAL(valueChanged(double)),
        this, SLOT(heightChanged(double)));
    connect(cmbColorSpaces, SIGNAL(activated(const KoID &)),
        this, SLOT(fillCmbProfiles(const KoID &)));
    connect (m_createButton, SIGNAL( clicked() ), this, SLOT (buttonClicked()) );
    m_createButton -> setDefault(true);

    fillCmbProfiles(cmbColorSpaces->currentItem());

}

void KisCustomImageWidget::widthUnitChanged(int index) {
    doubleWidth->blockSignals(true);

    if(index == KoUnit::Pixel) {
        doubleWidth->setDecimals(0);
        m_widthUnit = KoUnit(KoUnit::Pixel, doubleResolution->value() / 72.0);
    }
    else {
        doubleWidth->setDecimals(2);
        m_widthUnit = KoUnit((KoUnit::Unit)cmbWidthUnit->currentIndex());
    }

    doubleWidth->setValue(KoUnit::ptToUnit(m_width, m_widthUnit));

    doubleWidth->blockSignals(false);
}

void KisCustomImageWidget::widthChanged(double value) {
    m_width = KoUnit::fromUserValue(value, m_widthUnit);
}

void KisCustomImageWidget::heightUnitChanged(int index) {
    doubleHeight->blockSignals(true);

    if(index == KoUnit::Pixel) {
        doubleHeight->setDecimals(0);
        m_heightUnit = KoUnit(KoUnit::Pixel, doubleResolution->value()/72.0);
    }
    else {
        doubleHeight->setDecimals(2);
        m_heightUnit = KoUnit((KoUnit::Unit)cmbHeightUnit->currentIndex());
    }

    doubleHeight->setValue(KoUnit::ptToUnit(m_height, m_heightUnit));

    doubleHeight->blockSignals(false);
}

void KisCustomImageWidget::heightChanged(double value) {
    m_height = KoUnit::fromUserValue(value, m_heightUnit);
}

void KisCustomImageWidget::buttonClicked() {
    KoColorSpace * cs = KisMetaRegistry::instance()->csRegistry()->colorSpace(cmbColorSpaces->currentItem(), cmbProfile->currentText());

    QColor qc(cmbColor->color());

    qint32 width, height;
    double resolution;
    resolution =  doubleResolution->value() / 72.0;  // internal resolution is in pixels per pt

    // XXX: Added explicit casts to get rid of warning
    width = static_cast<qint32>(0.5  + KoUnit::ptToUnit(m_width, KoUnit(KoUnit::Pixel, resolution)));
    height = static_cast<qint32>(0.5 + KoUnit::ptToUnit(m_height, KoUnit(KoUnit::Pixel, resolution)));

    m_doc->newImage(txtName->text(), width, height, cs, KoColor(qc, cs), txtDescription->toPlainText(), resolution);

    KisImageSP img = m_doc->currentImage();
    if (img) {
        KisLayerSP layer = img->activeLayer();
        if (layer) {
            layer->setOpacity(backgroundOpacity());
        }
    }

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
