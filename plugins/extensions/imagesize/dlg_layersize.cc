/*
 *  dlg_layersize.cc - part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
 *  Copyright (c) 2013 Juan Palacios <jpalaciosdev@gmail.com>
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

#include "dlg_layersize.h"

#include <KoUnit.h>

#include <klocalizedstring.h>

#include <kis_filter_strategy.h>// XXX: I'm really real bad at arithmetic, let alone math. Here
// be rounding errors. (Boudewijn)

static const QString pixelStr(KoUnit::unitDescription(KoUnit::Pixel));
static const QString percentStr(i18n("Percent (%)"));

DlgLayerSize::DlgLayerSize(QWidget *  parent, const char * name,
                           int width, int height, double resolution)
        : KoDialog(parent)
        , m_aspectRatio(((double) width) / height)
        , m_originalWidth(width)
        , m_originalHeight(height)
        , m_width(width)
        , m_height(height)
        , m_resolution(resolution)
        , m_keepAspect(true)
{
    setCaption(i18n("Layer Size"));
    setObjectName(name);
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgLayerSize(this);
    Q_CHECK_PTR(m_page);
    m_page->layout()->setMargin(0);
    m_page->setObjectName(name);

    m_page->newWidth->setValue(width);
    m_page->newWidth->setFocus();
    m_page->newHeight->setValue(height);

    m_page->newWidthDouble->setVisible(false);
    m_page->newHeightDouble->setVisible(false);

    m_page->filterCmb->setIDList(KisFilterStrategyRegistry::instance()->listKeys());
    m_page->filterCmb->setToolTip(KisFilterStrategyRegistry::instance()->formatedDescriptions());
    m_page->filterCmb->setCurrent("Bicubic");

    m_page->newWidthUnit->addItems(KoUnit::listOfUnitNameForUi());
    m_page->newWidthUnit->addItem(percentStr);

    m_page->newHeightUnit->addItems(KoUnit::listOfUnitNameForUi());
    m_page->newHeightUnit->addItem(percentStr);

    const int pixelUnitIndex = KoUnit(KoUnit::Pixel).indexInListForUi();
    m_page->newWidthUnit->setCurrentIndex(pixelUnitIndex);
    m_page->newHeightUnit->setCurrentIndex(pixelUnitIndex);

    m_page->aspectRatioBtn->setKeepAspectRatio(true);
    m_page->constrainProportionsCkb->setChecked(true);

    setMainWidget(m_page);
    connect(this, SIGNAL(okClicked()), this, SLOT(accept()));

    connect(m_page->aspectRatioBtn, SIGNAL(keepAspectRatioChanged(bool)), this, SLOT(slotAspectChanged(bool)));
    connect(m_page->constrainProportionsCkb, SIGNAL(toggled(bool)), this, SLOT(slotAspectChanged(bool)));

    connect(m_page->newWidth, SIGNAL(valueChanged(int)), this, SLOT(slotWidthChanged(int)));
    connect(m_page->newHeight, SIGNAL(valueChanged(int)), this, SLOT(slotHeightChanged(int)));
    connect(m_page->newWidthDouble, SIGNAL(valueChanged(double)), this, SLOT(slotWidthChanged(double)));
    connect(m_page->newHeightDouble, SIGNAL(valueChanged(double)), this, SLOT(slotHeightChanged(double)));
    connect(m_page->newWidthUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotWidthUnitChanged(int)));
    connect(m_page->newHeightUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotHeightUnitChanged(int)));
}

DlgLayerSize::~DlgLayerSize()
{
    delete m_page;
}

qint32 DlgLayerSize::width()
{
    return (qint32)m_width;
}

qint32 DlgLayerSize::height()
{
    return (qint32)m_height;
}

KisFilterStrategy *DlgLayerSize::filterType()
{
    KoID filterID = m_page->filterCmb->currentItem();
    KisFilterStrategy *filter = KisFilterStrategyRegistry::instance()->value(filterID.id());
    return filter;
}

// SLOTS

void DlgLayerSize::slotWidthChanged(int w)
{
    slotWidthChanged((double) w);
}

void DlgLayerSize::slotHeightChanged(int h)
{
    slotHeightChanged((double) h);
}

void DlgLayerSize::slotWidthChanged(double w)
{
    if (m_page->newWidthUnit->currentText() == percentStr) {
        m_width = qRound((w * m_originalWidth) / 100.0);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->newWidthUnit->currentIndex());
        const double resValue = (selectedUnit == KoUnit(KoUnit::Pixel)) ? w : (w * m_resolution);
        m_width = qRound(selectedUnit.fromUserValue(resValue));
    }

    if (m_keepAspect) {
        m_height = qRound(m_width / m_aspectRatio);
        updateHeightUIValue(m_height);
    }
}

void DlgLayerSize::slotHeightChanged(double h)
{
    if (m_page->newHeightUnit->currentText() == percentStr) {
        m_height = qRound((h * m_originalHeight) / 100.0);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->newHeightUnit->currentIndex());
        const double resValue = (selectedUnit == KoUnit(KoUnit::Pixel)) ? h : (h * m_resolution);
        m_height = qRound(selectedUnit.fromUserValue(resValue));
    }

    if (m_keepAspect) {
        m_width = qRound(m_height * m_aspectRatio);
        updateWidthUIValue(m_width);
    }
}

void DlgLayerSize::slotWidthUnitChanged(int index)
{
    updateWidthUIValue(m_width);

    if (m_page->newWidthUnit->currentText() == percentStr) {
        m_page->newWidth->setVisible(false);
        m_page->newWidthDouble->setVisible(true);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(index);
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->newWidth->setVisible(false);
            m_page->newWidthDouble->setVisible(true);
        } else {
            m_page->newWidth->setVisible(true);
            m_page->newWidthDouble->setVisible(false);
        }
    }
}

void DlgLayerSize::slotHeightUnitChanged(int index)
{
    updateHeightUIValue(m_height);

    if (m_page->newHeightUnit->currentText() == percentStr) {
        m_page->newHeight->setVisible(false);
        m_page->newHeightDouble->setVisible(true);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(index);
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->newHeight->setVisible(false);
            m_page->newHeightDouble->setVisible(true);
        } else {
            m_page->newHeight->setVisible(true);
            m_page->newHeightDouble->setVisible(false);
        }
    }
}

void DlgLayerSize::slotAspectChanged(bool keep)
{
    m_page->aspectRatioBtn->blockSignals(true);
    m_page->constrainProportionsCkb->blockSignals(true);

    m_page->aspectRatioBtn->setKeepAspectRatio(keep);
    m_page->constrainProportionsCkb->setChecked(keep);

    m_page->aspectRatioBtn->blockSignals(false);
    m_page->constrainProportionsCkb->blockSignals(false);

    m_keepAspect = keep;

    if (keep) {
        // values may be out of sync, so we need to reset it to defaults
        m_width = m_originalWidth;
        m_height = m_originalHeight;

        updateWidthUIValue(m_width);
        updateHeightUIValue(m_height);
    }
}

void DlgLayerSize::updateWidthUIValue(double value)
{
    if (m_page->newWidthUnit->currentText() == percentStr) {
        m_page->newWidthDouble->blockSignals(true);
        m_page->newWidthDouble->setValue((value * 100.0) / m_originalWidth);
        m_page->newWidthDouble->blockSignals(false);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->newWidthUnit->currentIndex());
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->newWidthDouble->blockSignals(true);
            m_page->newWidthDouble->setValue(selectedUnit.toUserValue(value / m_resolution));
            m_page->newWidthDouble->blockSignals(false);
        } else {
            m_page->newWidth->blockSignals(true);
            m_page->newWidth->setValue(selectedUnit.toUserValue(value));
            m_page->newWidth->blockSignals(false);
        }
    }
}

void DlgLayerSize::updateHeightUIValue(double value)
{
    if (m_page->newHeightUnit->currentText() == percentStr) {
        m_page->newHeightDouble->blockSignals(true);
        m_page->newHeightDouble->setValue((value * 100.0) / m_originalHeight);
        m_page->newHeightDouble->blockSignals(false);
    } else {
        const KoUnit selectedUnit = KoUnit::fromListForUi(m_page->newHeightUnit->currentIndex());
        if (selectedUnit != KoUnit(KoUnit::Pixel)) {
            m_page->newHeightDouble->blockSignals(true);
            m_page->newHeightDouble->setValue(selectedUnit.toUserValue(value / m_resolution));
            m_page->newHeightDouble->blockSignals(false);
        } else {
            m_page->newHeight->blockSignals(true);
            m_page->newHeight->setValue(selectedUnit.toUserValue(value));
            m_page->newHeight->blockSignals(false);
        }
    }
}

