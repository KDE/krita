/*
 *  SPDX-FileCopyrightText: 2018 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
