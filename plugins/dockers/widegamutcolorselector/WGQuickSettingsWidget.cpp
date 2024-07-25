/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGQuickSettingsWidget.h"

#include "ui_WdgQuickSettings.h"
#include "WGConfigSelectorTypes.h"
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
#if QT_VERSION >= QT_VERSION_CHECK(5,15,0)
    connect(m_modelGroup, SIGNAL(idToggled(int,bool)), SLOT(slotColorGroupToggled(int,bool)));
#else
    connect(m_modelGroup, SIGNAL(buttonToggled(int,bool)), SLOT(slotColorGroupToggled(int,bool)));
#endif

    m_selectorConf = new WGSelectorConfigGrid(this);
    m_ui->verticalLayout->addWidget(m_selectorConf);
    connect(m_selectorConf, SIGNAL(sigConfigSelected(KisColorSelectorConfiguration)),
            SLOT(slotConfigSelected(KisColorSelectorConfiguration)));
}

WGQuickSettingsWidget::~WGQuickSettingsWidget()
{
    delete m_ui;
}

void WGQuickSettingsWidget::loadConfiguration()
{
    WGConfig::Accessor cfg;
    m_selectorConf->setConfigurations(cfg.favoriteConfigurations());
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
        m_selectorConf->setColorModel(m_selector->selectorModel()->colorModel());
        m_selectorConf->setChecked(m_selector->configuration());
    }
}

void WGQuickSettingsWidget::slotColorGroupToggled(int id, bool checked)
{
    if (!m_selector || !checked) {
        return;
    }
    KisVisualColorModel::ColorModel model = static_cast<KisVisualColorModel::ColorModel>(id);
    m_selector->selectorModel()->setRGBColorModel(model);
    m_selectorConf->setColorModel(model);

    WGConfig::Accessor cfg(false);
    cfg.set(WGConfig::rgbColorModel, model);
}

void WGQuickSettingsWidget::slotConfigSelected(const KisColorSelectorConfiguration &config)
{
    if (m_selector) {
        m_selector->setConfiguration(&config);
    }
    WGConfig::Accessor cfg(false);
    cfg.setColorSelectorConfiguration(config);
    WGConfig::notifier()->notifySelectorConfigChanged();
}
