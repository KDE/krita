/*
 *  Copyright (c) 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <kis_signals_blocker.h>

#include "KisLayerStyleAngleSelector.h"

KisLayerStyleAngleSelector::KisLayerStyleAngleSelector(QWidget *parent)
    : QWidget(parent)
    , m_enableGlobalLight(false)
{
    ui = new Ui_WdgKisLayerStyleAngleSelector();
    ui->setupUi(this);

    ui->angleSelector->setRange(-179.0, 180.0);
    ui->angleSelector->setDecimals(0);
    ui->angleSelector->setResetAngle(120.0);

    ui->chkUseGlobalLight->hide();

    connect(ui->angleSelector, SIGNAL(angleChanged(qreal)), SLOT(slotAngleSelectorAngleChanged(qreal)));
}

int KisLayerStyleAngleSelector::value()
{
    return static_cast<int>(ui->angleSelector->angle());
}

void KisLayerStyleAngleSelector::setValue(int value)
{
    KisSignalsBlocker angleSelectorBlocker(ui->angleSelector);

    ui->angleSelector->setAngle(static_cast<qreal>(value));
}

void KisLayerStyleAngleSelector::enableGlobalLight(bool enable)
{
    m_enableGlobalLight = enable;

    if (enable) {
        ui->chkUseGlobalLight->show();
        connect(ui->chkUseGlobalLight, SIGNAL(toggled(bool)), SLOT(slotGlobalLightToggled()));
    } else {
        ui->chkUseGlobalLight->hide();
        disconnect(ui->chkUseGlobalLight, SIGNAL(toggled(bool)), this, SLOT(slotGlobalLightToggled()));
    }
}

bool KisLayerStyleAngleSelector::useGlobalLight()
{
    return m_enableGlobalLight && ui->chkUseGlobalLight->isChecked();
}

void KisLayerStyleAngleSelector::setUseGlobalLight(bool state)
{
    ui->chkUseGlobalLight->setChecked(state);
}

KisAngleSelector* KisLayerStyleAngleSelector::angleSelector()
{
    return ui->angleSelector;
}

void KisLayerStyleAngleSelector::slotAngleSelectorAngleChanged(qreal value)
{
    emit valueChanged(static_cast<int>(value));
    emitChangeSignals();
}

void KisLayerStyleAngleSelector::slotGlobalLightToggled()
{
    emitChangeSignals();
}

void KisLayerStyleAngleSelector::emitChangeSignals()
{
    if (useGlobalLight()) {
        emit globalAngleChanged(value());
    }

    emit configChanged();
}
