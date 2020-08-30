/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGQuickSettingsWidget.h"

#include "ui_WdgQuickSettings.h"

#include <KisVisualColorSelector.h>

#include <QButtonGroup>

WGQuickSettingsWidget::WGQuickSettingsWidget(QWidget *parent, KisVisualColorSelector *selector)
    : QWidget(parent)
    , m_ui(new Ui_QuickSettingsWidget)
    , m_modelGroup(new QButtonGroup(this))
    , m_selector(selector)
{
    m_ui->setupUi(this);

    m_modelGroup->addButton(m_ui->btnHSV, KisVisualColorModel::HSV);
    m_modelGroup->addButton(m_ui->btnHSL, KisVisualColorModel::HSL);
    m_modelGroup->addButton(m_ui->btnHSI, KisVisualColorModel::HSI);
    m_modelGroup->addButton(m_ui->btnHSY, KisVisualColorModel::HSY);
    connect(m_modelGroup, SIGNAL(idToggled(int,bool)), SLOT(slotColorGroupToggled(int,bool)));
}

WGQuickSettingsWidget::~WGQuickSettingsWidget()
{
    delete m_ui;
}

void WGQuickSettingsWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    if (m_selector) {
        QAbstractButton *button = m_modelGroup->button(m_selector->selectorModel()->colorModel());
        if (button) {
            m_modelGroup->blockSignals(true);
            button->setChecked(true);
            m_modelGroup->blockSignals(false);
        }
    }
}

void WGQuickSettingsWidget::slotColorGroupToggled(int id, bool checked)
{
    if (!m_selector || !checked) {
        return;
    }
    KisVisualColorModel::ColorModel model = static_cast<KisVisualColorModel::ColorModel>(id);
    m_selector->selectorModel()->setColorModel(model);
    // TODO: write to config once there is one...
}
