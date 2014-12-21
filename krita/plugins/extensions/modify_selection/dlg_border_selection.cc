/*
 *  dlg_border_selection.cc - part of Krita
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

#include "dlg_border_selection.h"

#include <KoUnit.h>
#include <KoSizeGroup.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <operations/kis_operation_configuration.h>

WdgBorderSelection::WdgBorderSelection(QWidget* parent, KisViewManager *view)
    : KisOperationUIWidget(i18n("Border Selection"), parent)
    , m_width(1)
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
    KoSizeGroup *spbGroup = new KoSizeGroup(this);
    spbGroup->addWidget(spbWidth);
    spbGroup->addWidget(spbWidthDouble);

    connect(spbWidth, SIGNAL(valueChanged(int)), this, SLOT(slotWidthChanged(int)));
    connect(spbWidthDouble, SIGNAL(valueChanged(double)), this, SLOT(slotWidthChanged(double)));
    connect(cmbUnit, SIGNAL(currentIndexChanged(int)), this, SLOT(slotUnitChanged(int)));
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

void WdgBorderSelection::getConfiguration(KisOperationConfiguration* config)
{
    config->setProperty("x-radius", m_width);
    config->setProperty("y-radius", m_width);
}

#include "dlg_border_selection.moc"
