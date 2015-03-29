/*
 *  dlg_grow_selection.cc - part of Krita
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

#include "dlg_grow_selection.h"

#include <KoUnit.h>
#include <KoSizeGroup.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <operations/kis_operation_configuration.h>

WdgGrowSelection::WdgGrowSelection(QWidget* parent, KisViewManager* view)
    : KisOperationUIWidget(i18n("Grow Selection"), parent)
    , m_growValue(1)
{
    Q_ASSERT(view);
    KisImageWSP image = view->image();
    Q_ASSERT(image);
    m_resolution = image->yRes();

    setupUi(this);

    spbGrowValue->setValue(m_growValue);
    spbGrowValue->setFocus();
    spbGrowValue->setVisible(true);
    spbGrowValueDouble->setVisible(false);

    cmbUnit->addItems(KoUnit::listOfUnitNameForUi());
    cmbUnit->setCurrentIndex(KoUnit(KoUnit::Pixel).indexInListForUi());

    // ensure that both spinboxes request the same horizontal size
    KoSizeGroup *spbGroup = new KoSizeGroup(this);
    spbGroup->addWidget(spbGrowValue);
    spbGroup->addWidget(spbGrowValueDouble);

    connect(spbGrowValue, SIGNAL(valueChanged(int)), this, SLOT(slotGrowValueChanged(int)));
    connect(spbGrowValueDouble, SIGNAL(valueChanged(double)), this, SLOT(slotGrowValueChanged(double)));
    connect(cmbUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUnitChanged(int)));
}

void WdgGrowSelection::slotGrowValueChanged(int value)
{
    slotGrowValueChanged((double) value);
}

void WdgGrowSelection::slotGrowValueChanged(double value)
{
    const KoUnit selectedUnit = KoUnit::fromListForUi(cmbUnit->currentIndex());
    const double resValue = (selectedUnit == KoUnit(KoUnit::Pixel)) ? value : (value * m_resolution);
    m_growValue = qRound(selectedUnit.fromUserValue(resValue));
}

void WdgGrowSelection::slotUnitChanged(int index)
{
    updateGrowUIValue(m_growValue);

    const KoUnit selectedUnit = KoUnit::fromListForUi(index);
    if (selectedUnit != KoUnit(KoUnit::Pixel)) {
        spbGrowValue->setVisible(false);
        spbGrowValueDouble->setVisible(true);
    } else {
        spbGrowValue->setVisible(true);
        spbGrowValueDouble->setVisible(false);
    }
}

void WdgGrowSelection::updateGrowUIValue(double value)
{
    const KoUnit selectedUnit = KoUnit::fromListForUi(cmbUnit->currentIndex());
    if (selectedUnit != KoUnit(KoUnit::Pixel)) {
        spbGrowValueDouble->blockSignals(true);
        spbGrowValueDouble->setValue(selectedUnit.toUserValue(value / m_resolution));
        spbGrowValueDouble->blockSignals(false);
    } else {
        const int finalValue = (selectedUnit == KoUnit(KoUnit::Point)) ? qRound(value / m_resolution) : value;
        spbGrowValue->blockSignals(true);
        spbGrowValue->setValue(selectedUnit.toUserValue(finalValue));
        spbGrowValue->blockSignals(false);
    }
}

void WdgGrowSelection::getConfiguration(KisOperationConfiguration* config)
{
    config->setProperty("x-radius", m_growValue);
    config->setProperty("y-radius", m_growValue);
}

