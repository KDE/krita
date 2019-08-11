/*
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

#include "kis_rectangle_constraint_widget.h"
#include "kis_tool_rectangle_base.h"

#include <kis_icon.h>
#include "kis_aspect_ratio_locker.h"
#include "kis_signals_blocker.h"
#include <KConfigGroup>
#include <KSharedConfig>

KisRectangleConstraintWidget::KisRectangleConstraintWidget(QWidget *parent, KisToolRectangleBase *tool, bool showRoundCornersGUI)
    : QWidget(parent)
{
    m_tool = tool;
    
    setupUi(this);
    
    lockWidthButton->setIcon(KisIconUtils::loadIcon("layer-locked"));
    lockHeightButton->setIcon(KisIconUtils::loadIcon("layer-locked"));
    lockRatioButton->setIcon(KisIconUtils::loadIcon("layer-locked"));
    connect(lockWidthButton, SIGNAL(toggled(bool)), this, SLOT(inputsChanged()));
    connect(lockHeightButton, SIGNAL(toggled(bool)), this, SLOT(inputsChanged()));
    connect(lockRatioButton, SIGNAL(toggled(bool)), this, SLOT(inputsChanged()));

    connect(intWidth,  SIGNAL(valueChanged(int)), this, SLOT(inputsChanged()));
    connect(intHeight, SIGNAL(valueChanged(int)), this, SLOT(inputsChanged()));
    connect(doubleRatio, SIGNAL(valueChanged(double)), this, SLOT(inputsChanged()));
  
    connect(this, SIGNAL(constraintsChanged(bool,bool,bool,float,float,float)), m_tool, SLOT(constraintsChanged(bool,bool,bool,float,float,float)));
    connect(m_tool, SIGNAL(rectangleChanged(QRectF)), this, SLOT(rectangleChanged(QRectF)));

    m_cornersAspectLocker = new KisAspectRatioLocker(this);
    m_cornersAspectLocker->connectSpinBoxes(intRoundCornersX, intRoundCornersY, cornersAspectButton);

    connect(m_cornersAspectLocker, SIGNAL(sliderValueChanged()), SLOT(slotRoundCornersChanged()));
    connect(m_cornersAspectLocker, SIGNAL(aspectButtonChanged()), SLOT(slotRoundCornersAspectLockChanged()));

    connect(m_tool, SIGNAL(sigRequestReloadConfig()), SLOT(slotReloadConfig()));
    slotReloadConfig();

    if (!showRoundCornersGUI) {
        intRoundCornersX->setVisible(false);
        intRoundCornersY->setVisible(false);
        lblRoundCornersX->setVisible(false);
        lblRoundCornersY->setVisible(false);
        cornersAspectButton->setVisible(false);
    }
}

void KisRectangleConstraintWidget::inputsChanged() 
{
    emit constraintsChanged(
        lockRatioButton->isChecked(),
        lockWidthButton->isChecked(),
        lockHeightButton->isChecked(),
        doubleRatio->value(), 
        intWidth->value(), 
        intHeight->value()
                );
}

void KisRectangleConstraintWidget::slotRoundCornersChanged()
{
    m_tool->roundCornersChanged(intRoundCornersX->value(), intRoundCornersY->value());

    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_tool->toolId());
    cfg.writeEntry("roundCornersX", intRoundCornersX->value());
    cfg.writeEntry("roundCornersY", intRoundCornersY->value());
}

void KisRectangleConstraintWidget::slotRoundCornersAspectLockChanged()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_tool->toolId());
    cfg.writeEntry("roundCornersAspectLocked", cornersAspectButton->keepAspectRatio());
}

void KisRectangleConstraintWidget::slotReloadConfig()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(m_tool->toolId());

    {
        KisSignalsBlocker b(intRoundCornersX, intRoundCornersY, cornersAspectButton);
        intRoundCornersX->setValue(cfg.readEntry("roundCornersX", 0));
        intRoundCornersY->setValue(cfg.readEntry("roundCornersY", 0));
        cornersAspectButton->setKeepAspectRatio(cfg.readEntry("roundCornersAspectLocked", true));
        m_cornersAspectLocker->updateAspect();
    }

    slotRoundCornersChanged();
}

void KisRectangleConstraintWidget::rectangleChanged(const QRectF &rect) 
{
    intWidth->blockSignals(true);
    intHeight->blockSignals(true);
    doubleRatio->blockSignals(true);
    
    if (!lockWidthButton->isChecked()) intWidth->setValue(rect.width());
    if (!lockHeightButton->isChecked()) intHeight->setValue(rect.height());

    if (!lockRatioButton->isChecked() && !(rect.width() == 0 && rect.height() == 0)) {
        doubleRatio->setValue(fabs(rect.width()) / fabs(rect.height()));
    }
  
    intWidth->blockSignals(false);
    intHeight->blockSignals(false);
    doubleRatio->blockSignals(false);
}
