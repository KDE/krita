/*
 *  dlg_shrink_selection.cc - part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dlg_shrink_selection.h"

#include <KoUnit.h>
#include <kis_size_group.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <operations/kis_operation_configuration.h>

WdgShrinkSelection::WdgShrinkSelection(QWidget* parent, KisViewManager *view)
    : KisOperationUIWidget(i18n("Shrink Selection"), parent)
    , m_shrinkValue(1)
{
    Q_ASSERT(view);
    KisImageWSP image = view->image();
    Q_ASSERT(image);
    m_resolution = image->yRes();

    setupUi(this);

    spbShrinkValue->setValue(m_shrinkValue);
    spbShrinkValue->setFocus();
    spbShrinkValue->setVisible(true);
    spbShrinkValueDouble->setVisible(false);

    cmbUnit->addItems(KoUnit::listOfUnitNameForUi());
    cmbUnit->setCurrentIndex(KoUnit(KoUnit::Pixel).indexInListForUi());

    // ensure that both spinboxes request the same horizontal size
    KisSizeGroup *spbGroup = new KisSizeGroup(this);
    spbGroup->addWidget(spbShrinkValue);
    spbGroup->addWidget(spbShrinkValueDouble);

    connect(spbShrinkValue, SIGNAL(valueChanged(int)), this, SLOT(slotShrinkValueChanged(int)));
    connect(spbShrinkValueDouble, SIGNAL(valueChanged(double)), this, SLOT(slotShrinkValueChanged(double)));
    connect(cmbUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUnitChanged(int)));
}

void WdgShrinkSelection::slotShrinkValueChanged(int value)
{
    slotShrinkValueChanged((double) value);
}

void WdgShrinkSelection::slotShrinkValueChanged(double value)
{
    const KoUnit selectedUnit = KoUnit::fromListForUi(cmbUnit->currentIndex());
    const double resValue = (selectedUnit == KoUnit(KoUnit::Pixel)) ? value : (value * m_resolution);
    m_shrinkValue = qRound(selectedUnit.fromUserValue(resValue));
}

void WdgShrinkSelection::slotUnitChanged(int index)
{
    updateShrinkUIValue(m_shrinkValue);

    const KoUnit selectedUnit = KoUnit::fromListForUi(index);
    if (selectedUnit != KoUnit(KoUnit::Pixel)) {
        spbShrinkValue->setVisible(false);
        spbShrinkValueDouble->setVisible(true);
    } else {
        spbShrinkValue->setVisible(true);
        spbShrinkValueDouble->setVisible(false);
    }
}

void WdgShrinkSelection::updateShrinkUIValue(double value)
{
    const KoUnit selectedUnit = KoUnit::fromListForUi(cmbUnit->currentIndex());
    if (selectedUnit != KoUnit(KoUnit::Pixel)) {
        spbShrinkValueDouble->blockSignals(true);
        spbShrinkValueDouble->setValue(selectedUnit.toUserValue(value / m_resolution));
        spbShrinkValueDouble->blockSignals(false);
    } else {
        const int finalValue = (selectedUnit == KoUnit(KoUnit::Point)) ? qRound(value / m_resolution) : value;
        spbShrinkValue->blockSignals(true);
        spbShrinkValue->setValue(selectedUnit.toUserValue(finalValue));
        spbShrinkValue->blockSignals(false);
    }
}

void WdgShrinkSelection::getConfiguration(KisOperationConfigurationSP config)
{
    config->setProperty("x-radius", m_shrinkValue);
    config->setProperty("y-radius", m_shrinkValue);
    config->setProperty("edgeLock", !ckbShrinkFromImageBorder->isChecked());
}

