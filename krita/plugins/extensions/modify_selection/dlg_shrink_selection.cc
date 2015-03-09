/*
 *  dlg_shrink_selection.cc - part of Krita
 *
 *  Copyright (c) 2006 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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

#include "dlg_shrink_selection.h"

#include <KoUnit.h>
#include <KoSizeGroup.h>
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
    KoSizeGroup *spbGroup = new KoSizeGroup(this);
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

void WdgShrinkSelection::getConfiguration(KisOperationConfiguration* config)
{
    config->setProperty("x-radius", m_shrinkValue);
    config->setProperty("y-radius", m_shrinkValue);
    config->setProperty("edgeLock", !ckbShrinkFromImageBorder->isChecked());
}

#include "dlg_shrink_selection.moc"
