/*
 *  dlg_border_selection.cc - part of Krita
 *
 *  SPDX-FileCopyrightText: 2006 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  SPDX-FileCopyrightText: 2013 Juan Palacios <jpalaciosdev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dlg_border_selection.h"

#include <KoUnit.h>
#include <kis_size_group.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <operations/kis_operation_configuration.h>


WdgBorderSelection::WdgBorderSelection(QWidget* parent, KisViewManager *view, KisOperationConfigurationSP config)
    : KisOperationUIWidget(i18n("Border Selection"), parent)
    , m_width(config->getInt("x-radius", 1))
    , m_antialiasing(config->getBool("antialiasing", false))
{
    Q_ASSERT(view);
    KisImageWSP image = view->image();
    Q_ASSERT(image);
    m_resolution = image->yRes();

    setupUi(this);

    spbWidth->setValue(m_width);
    spbWidth->setFocus();
    spbWidth->setVisible(true);
    spbWidthDouble->setVisible(false);

    cmbUnit->addItems(KoUnit::listOfUnitNameForUi());
    cmbUnit->setCurrentIndex(KoUnit(KoUnit::Pixel).indexInListForUi());

    // ensure that both spinboxes request the same horizontal size
    KisSizeGroup *spbGroup = new KisSizeGroup(this);
    spbGroup->addWidget(spbWidth);
    spbGroup->addWidget(spbWidthDouble);

    connect(spbWidth, SIGNAL(valueChanged(int)), this, SLOT(slotWidthChanged(int)));
    connect(spbWidthDouble, SIGNAL(valueChanged(double)), this, SLOT(slotWidthChanged(double)));
    connect(cmbUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUnitChanged(int)));
    connect(chkAntialiasing, SIGNAL(toggled(bool)), this, SLOT(slotAntialiasingChanged(bool)));
    slotUpdateAntialiasingAvailability();
}

void WdgBorderSelection::slotWidthChanged(int width)
{
    slotWidthChanged((double) width);
}

void WdgBorderSelection::slotWidthChanged(double width)
{
    const KoUnit selectedUnit = KoUnit::fromListForUi(cmbUnit->currentIndex());
    const double resWidth = (selectedUnit == KoUnit(KoUnit::Pixel)) ? width : (width * m_resolution);
    m_width = qRound(selectedUnit.fromUserValue(resWidth));
    slotUpdateAntialiasingAvailability();
}

void WdgBorderSelection::slotUnitChanged(int index)
{
    updateWidthUIValue(m_width);

    const KoUnit selectedUnit = KoUnit::fromListForUi(index);
    if (selectedUnit != KoUnit(KoUnit::Pixel)) {
        spbWidth->setVisible(false);
        spbWidthDouble->setVisible(true);
    } else {
        spbWidth->setVisible(true);
        spbWidthDouble->setVisible(false);
    }
}

void WdgBorderSelection::slotAntialiasingChanged(bool value)
{
    m_antialiasing = value;
}

void WdgBorderSelection::slotUpdateAntialiasingAvailability()
{
    const bool antialiasingEnabled = m_width > 1;

    if (antialiasingEnabled) {
        chkAntialiasing->setChecked(m_antialiasing);
    } else {
        bool tmp_antialiasing = m_antialiasing;
        chkAntialiasing->setChecked(false);
        m_antialiasing = tmp_antialiasing;
    }

    chkAntialiasing->setEnabled(antialiasingEnabled);
}

void WdgBorderSelection::updateWidthUIValue(double value)
{
    const KoUnit selectedUnit = KoUnit::fromListForUi(cmbUnit->currentIndex());
    if (selectedUnit != KoUnit(KoUnit::Pixel)) {
        spbWidthDouble->blockSignals(true);
        spbWidthDouble->setValue(selectedUnit.toUserValue(value / m_resolution));
        spbWidthDouble->blockSignals(false);
    } else {
        const int finalValue = (selectedUnit == KoUnit(KoUnit::Point)) ? qRound(value / m_resolution) : value;
        spbWidth->blockSignals(true);
        spbWidth->setValue(selectedUnit.toUserValue(finalValue));
        spbWidth->blockSignals(false);
    }
}

void WdgBorderSelection::getConfiguration(KisOperationConfigurationSP config)
{
    config->setProperty("x-radius", m_width);
    config->setProperty("y-radius", m_width);
    config->setProperty("antialiasing", m_antialiasing);
}

