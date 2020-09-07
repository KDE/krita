/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGQuickSettingsWidget.h"

#include "ui_WdgQuickSettings.h"
#include "WGSelectorConfigGrid.h"

#include <KisVisualColorSelector.h>
#include <KisColorSelectorConfiguration.h>

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

    m_selectorConf = new WGSelectorConfigGrid(this);
    // test configuration
    QVector<KisColorSelectorConfiguration> confs;
    confs.append(KisColorSelectorConfiguration());
    confs.append(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Square, KisColorSelectorConfiguration::Ring, KisColorSelectorConfiguration::SV, KisColorSelectorConfiguration::H));
    confs.append(KisColorSelectorConfiguration(KisColorSelectorConfiguration::Wheel, KisColorSelectorConfiguration::Slider, KisColorSelectorConfiguration::VH, KisColorSelectorConfiguration::hsvS));
    m_selectorConf->setConfigurations(confs);
    if (layout()) {
        layout()->addWidget(m_selectorConf);
    }
    connect(m_selectorConf, SIGNAL(sigConfigSelected(KisColorSelectorConfiguration)),
            SLOT(slotConfigSelected(KisColorSelectorConfiguration)));
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
    m_selectorConf->setColorModel(m_selector->selectorModel()->colorModel());
}

void WGQuickSettingsWidget::slotColorGroupToggled(int id, bool checked)
{
    if (!m_selector || !checked) {
        return;
    }
    KisVisualColorModel::ColorModel model = static_cast<KisVisualColorModel::ColorModel>(id);
    m_selector->selectorModel()->setColorModel(model);
    m_selectorConf->setColorModel(model);
    // TODO: write to config once there is one...
}

void WGQuickSettingsWidget::slotConfigSelected(const KisColorSelectorConfiguration &cfg)
{
    if (m_selector) {
        m_selector->setConfiguration(&cfg);
    }
}
