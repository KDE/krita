/*
 *  dlg_feather_selection.cc - part of Krita
 *
 *  Copyright (c) 2009 Edward Apap <schumifer@hotmail.com>
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

#include "dlg_feather_selection.h"

#include <KoUnit.h>
#include <kis_size_group.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <operations/kis_operation_configuration.h>

WdgFeatherSelection::WdgFeatherSelection(QWidget* parent, KisViewManager* view)
    : KisOperationUIWidget(i18n("Feather Selection"), parent)
    , m_radius(5)
{
    Q_ASSERT(view);
    KisImageWSP image = view->image();
    Q_ASSERT(image);
    m_resolution = image->yRes();

    setupUi(this);

    spbRadius->setValue(m_radius);
    spbRadius->setFocus();
    spbRadius->setVisible(true);
    spbRadiusDouble->setVisible(false);

    cmbUnit->addItems(KoUnit::listOfUnitNameForUi());
    cmbUnit->setCurrentIndex(KoUnit(KoUnit::Pixel).indexInListForUi());

    // ensure that both spinboxes request the same horizontal size
    KisSizeGroup *spbGroup = new KisSizeGroup(this);
    spbGroup->addWidget(spbRadius);
    spbGroup->addWidget(spbRadiusDouble);

    connect(spbRadius, SIGNAL(valueChanged(int)), this, SLOT(slotRadiusChanged(int)));
    connect(spbRadiusDouble, SIGNAL(valueChanged(double)), this, SLOT(slotRadiusChanged(double)));
    connect(cmbUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUnitChanged(int)));
}

void WdgFeatherSelection::slotRadiusChanged(int radius)
{
    slotRadiusChanged((double) radius);
}

void WdgFeatherSelection::slotRadiusChanged(double radius)
{
    const KoUnit selectedUnit = KoUnit::fromListForUi(cmbUnit->currentIndex());
    const double resRadius = (selectedUnit == KoUnit(KoUnit::Pixel)) ? radius : (radius * m_resolution);
    m_radius = qRound(selectedUnit.fromUserValue(resRadius));
}

void WdgFeatherSelection::slotUnitChanged(int index)
{
    updateRadiusUIValue(m_radius);

    const KoUnit selectedUnit = KoUnit::fromListForUi(index);
    if (selectedUnit != KoUnit(KoUnit::Pixel)) {
        spbRadius->setVisible(false);
        spbRadiusDouble->setVisible(true);
    } else {
        spbRadius->setVisible(true);
        spbRadiusDouble->setVisible(false);
    }
}

void WdgFeatherSelection::updateRadiusUIValue(double value)
{
    const KoUnit selectedUnit = KoUnit::fromListForUi(cmbUnit->currentIndex());
    if (selectedUnit != KoUnit(KoUnit::Pixel)) {
        spbRadiusDouble->blockSignals(true);
        spbRadiusDouble->setValue(selectedUnit.toUserValue(value / m_resolution));
        spbRadiusDouble->blockSignals(false);
    } else {
        const int finalValue = (selectedUnit == KoUnit(KoUnit::Point)) ? qRound(value / m_resolution) : value;
        spbRadius->blockSignals(true);
        spbRadius->setValue(selectedUnit.toUserValue(finalValue));
        spbRadius->blockSignals(false);
    }
}

void WdgFeatherSelection::getConfiguration(KisOperationConfiguration* config)
{
    config->setProperty("radius", m_radius);
}

