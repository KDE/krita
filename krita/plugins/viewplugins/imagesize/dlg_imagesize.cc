/*
 *  dlg_imagesize.cc - part of KimageShop^WKrayon^WKrita
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

#include <math.h>

#include <klocale.h>
#include <kdebug.h>

#include <kis_cmb_idlist.h>
#include <kis_filter_strategy.h>

#include "dlg_imagesize.h"

// XXX: I'm really real bad at arithmetic, let alone math. Here
// be rounding errors. (Boudewijn)
DlgImageSize::DlgImageSize(QWidget *parent, int width, int height)
    : super(parent)
{
    setCaption( i18n("Scale To New Size") );
    setButtons( Ok | Cancel);
    setDefaultButton( Ok );

    m_origW = width;
    m_origH = height;

    m_page = new WdgImageSize(this);
    Q_CHECK_PTR(m_page);
    m_page->setObjectName("image_size");

    m_page->intPixelWidth->setValue(width);
    m_page->intPixelHeight->setValue(height);

    m_page->cmbFilterType->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    m_page->cmbFilterType->setCurrent("Mitchell");

    m_buttonGroup = new QButtonGroup(m_page);
    m_buttonGroup->addButton(m_page->radioProtectPixel);
    m_buttonGroup->addButton(m_page->radioProtectPhysical);
    m_buttonGroup->addButton(m_page->radioProtectResolution);

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    connect(m_page->radioProtectPixel, SIGNAL(toggled(bool)),
        this, SLOT(slotProtectChanged()));

    connect(m_page->radioProtectPhysical, SIGNAL(toggled(bool)),
        this, SLOT(slotProtectChanged()));

    connect(m_page->radioProtectResolution, SIGNAL(toggled(bool)),
        this, SLOT(slotProtectChanged()));

    m_page->radioProtectResolution->setChecked(true);

    connect(m_page->intPixelWidth, SIGNAL(valueChanged(int)),
        this, SLOT(slotWidthPixelsChanged(int)));

    connect(m_page->intPixelHeight, SIGNAL(valueChanged(int)),
        this, SLOT(slotHeightPixelsChanged(int)));

    connect(m_page->doublePhysicalWidth, SIGNAL(valueChanged(double)),
        this, SLOT(slotWidthPhysicalChanged(double)));

    connect(m_page->doublePhysicalHeight, SIGNAL(valueChanged(double)),
        this, SLOT(slotHeightPhysicalChanged(double)));

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

    if(m_page->radioProtectResolution->isChecked()) {
        m_page->doublePhysicalHeight->setValue(w*m_page->doubleResolution->value());
    }
    else {
//        m_page->doubleResolution->setValue(w*m_page->doubleResolution->value());
    }
    unblockAll();
}

void DlgImageSize::slotHeightPixelsChanged(int h)
{
    blockAll();

    // Set width in pixels and percent of necessary
/*    if (m_page->chkConstrain->isChecked()) {
        m_page->intWidth->setValue(qRound(m_oldW));

    }
*/
    unblockAll();
}

void DlgImageSize::slotWidthPhysicalChanged(double h)
{
    blockAll();

    unblockAll();
}

void DlgImageSize::slotHeightPhysicalChanged(double h)
{
    blockAll();

    unblockAll();
}

void DlgImageSize::slotProtectChanged()
{
    m_page->intPixelWidth->setEnabled(!m_page->radioProtectPixel->isChecked());
    m_page->intPixelHeight->setEnabled(!m_page->radioProtectPixel->isChecked());
    m_page->doublePhysicalWidth->setEnabled(!m_page->radioProtectPhysical->isChecked());
    m_page->doublePhysicalHeight->setEnabled(!m_page->radioProtectPhysical->isChecked());
    m_page->doubleResolution->setEnabled(!m_page->radioProtectResolution->isChecked());
}

void DlgImageSize::blockAll()
{
    m_page->intPixelWidth->blockSignals(true);
    m_page->intPixelHeight->blockSignals(true);
    m_page->doublePhysicalWidth->blockSignals(true);
    m_page->doublePhysicalHeight->blockSignals(true);
    m_page->doubleResolution->blockSignals(true);
}

void DlgImageSize::unblockAll()
{
    m_page->intPixelWidth->blockSignals(false);
    m_page->intPixelHeight->blockSignals(false);
    m_page->doublePhysicalWidth->blockSignals(false);
    m_page->doublePhysicalHeight->blockSignals(false);
    m_page->doubleResolution->blockSignals(false);
}

#include "dlg_imagesize.moc"
