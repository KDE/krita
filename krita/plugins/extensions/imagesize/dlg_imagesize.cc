/*
 *  dlg_imagesize.cc - part of KimageShop^WKrayon^WKrita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2009 Casper Boemann <cbr@boemann.dk>
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

#include "dlg_imagesize.h"

#include <math.h>

#include <klocale.h>
#include <kis_debug.h>

#include <KoUnit.h>

#include <widgets/kis_cmb_idlist.h>
#include <kis_filter_strategy.h>// XXX: I'm really real bad at arithmetic, let alone math. Here
// be rounding errors. (Boudewijn)

DlgImageSize::DlgImageSize(QWidget *parent, int width, int height, double resolution)
        : KDialog(parent)
{
    setCaption(i18n("Scale To New Size"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_origW = width;
    m_origH = height;

    m_width = width / resolution;
    m_height = height / resolution;

    m_aspectRatio = m_width / m_height;

    m_page = new WdgImageSize(this);
    m_page->layout()->setMargin(0);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("image_size");

    m_page->intPixelWidth->setValue(width);
    m_page->intPixelHeight->setValue(height);

    m_page->cmbFilterType->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    m_page->cmbFilterType->setCurrent("Bicubic");

    m_page->cmbWidthUnit->addItems(KoUnit::listOfUnitName());
    m_page->cmbHeightUnit->addItems(KoUnit::listOfUnitName());

    m_page->doubleResolution->setValue(72.0 * resolution);

    m_page->cmbInteractor->addItem(i18n("Size in Pixels"));
    m_page->cmbInteractor->addItem(i18n("Print Size"));

    slotAspectChanged(true);

    m_page->doublePercentageWidth->setVisible(false);
    m_page->doublePercentageHeight->setVisible(false);

    m_page->doublePercentageWidth->setValue(100.0 * width / m_origW);
    m_page->doublePercentageHeight->setValue(100.0 * height / m_origH);

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(m_page->chkAffectResolution, SIGNAL(toggled(bool)),
            this, SLOT(slotProtectChanged()));

    connect(m_page->aspectPixels, SIGNAL(keepAspectRatioChanged(bool)),
            this, SLOT(slotAspectChanged(bool)));

    connect(m_page->aspectPhysical, SIGNAL(keepAspectRatioChanged(bool)),
            this, SLOT(slotAspectChanged(bool)));

    connect(m_page->cmbInteractor, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotProtectChanged()));

    connect(m_page->intPixelWidth, SIGNAL(valueChanged(int)),
            this, SLOT(slotWidthPixelsChanged(int)));

    connect(m_page->intPixelHeight, SIGNAL(valueChanged(int)),
            this, SLOT(slotHeightPixelsChanged(int)));

    connect(m_page->doublePercentageWidth, SIGNAL(valueChanged(double)),
            this, SLOT(slotWidthPercentageChanged(double)));

    connect(m_page->doublePercentageHeight, SIGNAL(valueChanged(double)),
            this, SLOT(slotHeightPercentageChanged(double)));

    connect(m_page->cmbWidthPixelUnit, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotWidthPixelUnitChanged(int)));

    connect(m_page->cmbHeightPixelUnit, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotHeightPixelUnitChanged(int)));

    connect(m_page->doublePhysicalWidth, SIGNAL(valueChanged(double)),
            this, SLOT(slotWidthPhysicalChanged(double)));

    connect(m_page->doublePhysicalWidth, SIGNAL(valueChanged(double)),
            this, SLOT(slotWidthPhysicalChanged(double)));

    connect(m_page->doublePhysicalHeight, SIGNAL(valueChanged(double)),
            this, SLOT(slotHeightPhysicalChanged(double)));

    connect(m_page->cmbWidthUnit, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotWidthUnitChanged(int)));

    connect(m_page->cmbHeightUnit, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotHeightUnitChanged(int)));

    slotProtectChanged();

#ifdef __GNUC__
#warning "DlgImageSize: should take current units from a setting"
#endif

    m_page->cmbWidthUnit->setCurrentIndex(KoUnit::Centimeter);
    m_page->cmbHeightUnit->setCurrentIndex(KoUnit::Centimeter);

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));
}

DlgImageSize::~DlgImageSize()
{
    delete m_page;
}

qint32 DlgImageSize::width()
{
    return (qint32)m_page->intPixelWidth->value();
}

qint32 DlgImageSize::height()
{
    return (qint32)m_page->intPixelHeight->value();
}

double DlgImageSize::resolution()
{
    return m_page->doubleResolution->value() / 72.0;
}

KisFilterStrategy *DlgImageSize::filterType()
{
    KoID filterID = m_page->cmbFilterType->currentItem();
    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value(filterID.id());
    return filter;
}

// SLOTS

void DlgImageSize::okClicked()
{
    accept();
}

void DlgImageSize::slotWidthPixelsChanged(int w)
{
    blockAll();

    if (!m_page->chkAffectResolution->isChecked()) {
        m_width = 72 * w / m_page->doubleResolution->value();

        KoUnit unit = KoUnit((KoUnit::Unit)m_page->cmbWidthUnit->currentIndex());
        m_page->doublePhysicalWidth->setValue(unit.toUserValue(m_width));
    } else {
        m_page->doubleResolution->setValue(72 * w / m_width);
        // since we only have one resolution parameter we need to recalculate the height in pixels
        m_page->intPixelHeight->setValue(int(0.5 + m_height * m_page->doubleResolution->value() / 72.0));
    }

    if (m_page->aspectPixels->keepAspectRatio()) {
        m_height = m_width / m_aspectRatio;

        KoUnit unit = KoUnit((KoUnit::Unit)m_page->cmbHeightUnit->currentIndex());
        m_page->doublePhysicalHeight->setValue(unit.toUserValue(m_height));

        m_page->intPixelHeight->setValue(int(0.5 + m_height * m_page->doubleResolution->value() / 72.0));
    }

    // recalculate aspect ratio
    m_aspectRatio = m_width / m_height;

    if (m_page->cmbWidthPixelUnit->currentIndex() == 0)
        m_page->doublePercentageWidth->setValue(100.0 * m_page->intPixelWidth->value() / m_origW);
    m_page->doublePercentageHeight->setValue(100.0 * m_page->intPixelHeight->value() / m_origH);

    unblockAll();
}

void DlgImageSize::slotHeightPixelsChanged(int h)
{
    blockAll();

    if (!m_page->chkAffectResolution->isChecked()) {
        m_height = 72 * h / m_page->doubleResolution->value();

        KoUnit unit = KoUnit((KoUnit::Unit)m_page->cmbHeightUnit->currentIndex());
        m_page->doublePhysicalHeight->setValue(unit.toUserValue(m_height));
    } else {
        m_page->doubleResolution->setValue(72 * h / m_height);
        // since we only have one resolution parameter we need to recalculate the width in pixels
        m_page->intPixelWidth->setValue(int(0.5 + m_width * m_page->doubleResolution->value() / 72.0));
    }

    if (m_page->aspectPixels->keepAspectRatio()) {
        m_width = m_aspectRatio * m_height;

        KoUnit unit = KoUnit((KoUnit::Unit)m_page->cmbWidthUnit->currentIndex());
        m_page->doublePhysicalWidth->setValue(unit.toUserValue(m_width));

        m_page->intPixelWidth->setValue(int(0.5 + m_width * m_page->doubleResolution->value() / 72.0));
    }

    // recalculate aspect ratio
    m_aspectRatio = m_width / m_height;

    m_page->doublePercentageWidth->setValue(100.0 * m_page->intPixelWidth->value() / m_origW);
    if (m_page->cmbHeightPixelUnit->currentIndex() == 0)
        m_page->doublePercentageHeight->setValue(100.0 * m_page->intPixelHeight->value() / m_origH);

    unblockAll();
}

void DlgImageSize::slotWidthPercentageChanged(double w)
{
    m_page->intPixelWidth->setValue(w * m_origW / 100.0);
}

void DlgImageSize::slotHeightPercentageChanged(double h)
{
    m_page->intPixelHeight->setValue(h * m_origH / 100.0);
}

void DlgImageSize::slotWidthPixelUnitChanged(int index)
{
    m_page->intPixelWidth->setVisible(index == 0);
    m_page->doublePercentageWidth->setVisible(index == 1);

    //Make sure the percentage value is correct - which it might not have been have we 1)just entered a value 2)switched to pixel and back
    blockAll();
    if (index == 1)
        m_page->doublePercentageWidth->setValue(100.0 * m_page->intPixelWidth->value() / m_origW);
    unblockAll();
}

void DlgImageSize::slotHeightPixelUnitChanged(int index)
{
    m_page->intPixelHeight->setVisible(index == 0);
    m_page->doublePercentageHeight->setVisible(index == 1);

    //Make sure the percentage value is correct - which it might not have been have we 1)just entered a value 2)switched to pixel and back
    blockAll();
    if (index == 1)
        m_page->doublePercentageHeight->setValue(100.0 * m_page->intPixelHeight->value() / m_origH);
    unblockAll();
}

void DlgImageSize::slotWidthPhysicalChanged(double w)
{
    blockAll();

    KoUnit unit = KoUnit((KoUnit::Unit)m_page->cmbWidthUnit->currentIndex());
    m_width = unit.fromUserValue(w);

    if (!m_page->chkAffectResolution->isChecked()) {
        m_page->intPixelWidth->setValue(int(0.5 + m_width*m_page->doubleResolution->value() / 72.0));
    } else {
        m_page->doubleResolution->setValue(72*m_page->intPixelWidth->value() / m_width);
        // since we only have one resolution parameter we need to recalculate the physical height
        m_height = 72 * m_page->intPixelHeight->value() / m_page->doubleResolution->value();

        unit = KoUnit((KoUnit::Unit)m_page->cmbHeightUnit->currentIndex());
        m_page->doublePhysicalHeight->setValue(unit.toUserValue(m_height));
    }

    if (m_page->aspectPixels->keepAspectRatio()) {
        m_height = m_width / m_aspectRatio;

        unit = KoUnit((KoUnit::Unit)m_page->cmbHeightUnit->currentIndex());
        m_page->doublePhysicalHeight->setValue(unit.toUserValue(m_height));

        m_page->intPixelHeight->setValue(int(0.5 + m_height * m_page->doubleResolution->value() / 72.0));
    }

    // recalculate aspect ratio
    m_aspectRatio = m_width / m_height;

    unblockAll();
}

void DlgImageSize::slotHeightPhysicalChanged(double h)
{
    blockAll();

    KoUnit unit = KoUnit((KoUnit::Unit)m_page->cmbHeightUnit->currentIndex());
    m_height = unit.fromUserValue(h);

    if (!m_page->chkAffectResolution->isChecked()) {
        m_page->intPixelHeight->setValue(int(0.5 + m_height*m_page->doubleResolution->value() / 72.0));
    } else {
        m_page->doubleResolution->setValue(72*m_page->intPixelHeight->value() / m_height);

        // since we only have one resolution parameter we need to recalculate the physical width
        m_width = 72 * m_page->intPixelWidth->value() / m_page->doubleResolution->value();

        unit = KoUnit((KoUnit::Unit)m_page->cmbWidthUnit->currentIndex());
        m_page->doublePhysicalWidth->setValue(unit.toUserValue(m_width));
    }

    if (m_page->aspectPixels->keepAspectRatio()) {
        m_width = m_aspectRatio * m_height;

        unit = KoUnit((KoUnit::Unit)m_page->cmbWidthUnit->currentIndex());
        m_page->doublePhysicalWidth->setValue(unit.toUserValue(m_width));

        m_page->intPixelWidth->setValue(int(0.5 + m_width * m_page->doubleResolution->value() / 72.0));
    }

    // recalculate aspect ratio
    m_aspectRatio = m_width / m_height;

    unblockAll();
}

void DlgImageSize::slotWidthUnitChanged(int index)
{
    blockAll();

    KoUnit unit = KoUnit((KoUnit::Unit)index);
    m_page->doublePhysicalWidth->setValue(unit.toUserValue(m_width));


    unblockAll();
}

void DlgImageSize::slotHeightUnitChanged(int index)
{
    blockAll();

    KoUnit unit = KoUnit((KoUnit::Unit)index);
    m_page->doublePhysicalHeight->setValue(unit.toUserValue(m_height));

    unblockAll();
}

void DlgImageSize::slotProtectChanged()
{
    if (m_page->chkAffectResolution->isChecked()) {
        m_page->intPixelWidth->setEnabled(m_page->cmbInteractor->currentIndex() == 0);
        m_page->intPixelHeight->setEnabled(m_page->cmbInteractor->currentIndex() == 0);
        m_page->doublePhysicalWidth->setEnabled(m_page->cmbInteractor->currentIndex() == 1);
        m_page->doublePhysicalHeight->setEnabled(m_page->cmbInteractor->currentIndex() == 1);
        m_page->doubleResolution->setEnabled(true);
        m_page->cmbInteractor->setEnabled(true);
        m_page->labelInteractor->setEnabled(true);
    } else {
        m_page->intPixelWidth->setEnabled(true);
        m_page->intPixelHeight->setEnabled(true);
        m_page->doublePhysicalWidth->setEnabled(true);
        m_page->doublePhysicalHeight->setEnabled(true);
        m_page->doubleResolution->setEnabled(false);
        m_page->cmbInteractor->setEnabled(false);
        m_page->labelInteractor->setEnabled(false);
    }
}

void DlgImageSize::slotAspectChanged(bool keep)
{
    m_page->aspectPixels->setKeepAspectRatio(keep);
    m_page->aspectPhysical->setKeepAspectRatio(keep);
}

void DlgImageSize::blockAll()
{
    m_page->intPixelWidth->blockSignals(true);
    m_page->intPixelHeight->blockSignals(true);
    m_page->doublePhysicalWidth->blockSignals(true);
    m_page->doublePhysicalHeight->blockSignals(true);
    m_page->doubleResolution->blockSignals(true);
    m_page->doublePercentageWidth->blockSignals(true);
    m_page->doublePercentageHeight->blockSignals(true);
}

void DlgImageSize::unblockAll()
{
    m_page->intPixelWidth->blockSignals(false);
    m_page->intPixelHeight->blockSignals(false);
    m_page->doublePhysicalWidth->blockSignals(false);
    m_page->doublePhysicalHeight->blockSignals(false);
    m_page->doubleResolution->blockSignals(false);
    m_page->doublePercentageWidth->blockSignals(false);
    m_page->doublePercentageHeight->blockSignals(false);
}

#include "dlg_imagesize.moc"
