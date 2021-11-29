/*
 *  dlg_feather_selection.cc - part of Krita
 *
 *  SPDX-FileCopyrightText: 2009 Edward Apap <schumifer@hotmail.com>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dlg_feather_selection.h"

#include <KoUnit.h>
#include <kis_size_group.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <operations/kis_operation_configuration.h>


WdgFeatherSelection::WdgFeatherSelection(QWidget* parent, KisViewManager* view, KisOperationConfigurationSP config)
    : KisOperationUIWidget(i18n("Feather Selection"), parent)
    , m_radius(config->getInt("radius", 5))
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

void WdgFeatherSelection::getConfiguration(KisOperationConfigurationSP config)
{
    config->setProperty("radius", m_radius);
}
