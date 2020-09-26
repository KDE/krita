/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorSelectorSettings.h"
#include "ui_WdgWGSelectorSettings.h"

#include "WGConfig.h"
#include "WGSelectorConfigGrid.h"

#include "kis_config.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QStringList>
#include <QPushButton>

WGColorSelectorSettings::WGColorSelectorSettings(QWidget *parent)
    : KisPreferenceSet(parent)
    , m_ui(new Ui::WGConfigWidget)
{
    m_ui->setupUi(this);
    m_selectorConfigGrid = new WGSelectorConfigGrid;
    m_selectorConfigGrid->setConfigurations(WGSelectorConfigGrid::hueBasedConfigurations());
    m_ui->btnSelectorShape->setPopupWidget(m_selectorConfigGrid);
    connect(m_selectorConfigGrid, SIGNAL(sigConfigSelected(KisColorSelectorConfiguration)),
            SLOT(slotSetSelectorConfiguration(KisColorSelectorConfiguration)));
    connect(m_selectorConfigGrid, SIGNAL(sigConfigSelected(KisColorSelectorConfiguration)),
            m_ui->btnSelectorShape, SLOT(hidePopupWidget()));
    connect(m_ui->cmbColorModel, SIGNAL(currentIndexChanged(int)), SLOT(slotSetColorModel(int)));
    m_favoriteConfigGrid = new WGSelectorConfigGrid(0, true);
    m_favoriteConfigGrid->setConfigurations(WGSelectorConfigGrid::hueBasedConfigurations());
    m_ui->btnFavoriteSelectors->setPopupWidget(m_favoriteConfigGrid);
}

WGColorSelectorSettings::~WGColorSelectorSettings()
{
}

QString WGColorSelectorSettings::id()
{
    return stringID();
}

QString WGColorSelectorSettings::name()
{
    return QString(i18n("Wide Gamut Selector"));
}

QString WGColorSelectorSettings::header()
{
    return QString(i18n("Wide Gamut Color Selector Settings"));
}

QIcon WGColorSelectorSettings::icon()
{
    return QIcon();
}

QString WGColorSelectorSettings::stringID()
{
    return QString("WideGamutColorSelector");
}

void WGColorSelectorSettings::savePreferences() const
{
    WGConfig cfg(false);
    cfg.writeEntry("renderMode", m_ui->cmbSelectorRenderingMode->currentIndex());
    cfg.writeEntry("rgbColorModel", m_ui->cmbColorModel->currentIndex() +  KisVisualColorModel::HSV);
    cfg.writeEntry("colorSelectorConfiguration", m_selectorConfigGrid->currentConfiguration().toString());
    cfg.setFavoriteConfigurations(m_favoriteConfigGrid->selectedConfigurations());
}

void WGColorSelectorSettings::loadPreferences()
{
    WGConfig cfg;
    m_ui->cmbSelectorRenderingMode->setCurrentIndex(cfg.readEntry("renderMode", 1));
    m_ui->cmbColorModel->setCurrentIndex(cfg.readEntry("rgbColorModel", 2) - KisVisualColorModel::HSV);
    KisColorSelectorConfiguration selectorCfg(cfg.readEntry<QString>("colorSelectorConfiguration", "3|0|6|0")); // triangle selector
    m_selectorConfigGrid->setChecked(selectorCfg);
    m_ui->btnSelectorShape->setIcon(m_selectorConfigGrid->generateIcon(selectorCfg));
    QVector<KisColorSelectorConfiguration> favoriteConfigs = cfg.favoriteConfigurations();
    for (const KisColorSelectorConfiguration &fav: favoriteConfigs) {
        m_favoriteConfigGrid->setChecked(fav);
    }
}

void WGColorSelectorSettings::loadDefaultPreferences()
{

}

void WGColorSelectorSettings::slotSetSelectorConfiguration(const KisColorSelectorConfiguration &cfg)
{
    Q_UNUSED(cfg);
    m_ui->btnSelectorShape->setIcon(m_selectorConfigGrid->currentIcon());
}

void WGColorSelectorSettings::slotSetColorModel(int index)
{
    KisVisualColorModel::ColorModel model;
    switch (index) {
    case 0:
    default:
        model = KisVisualColorModel::HSV;
        break;
    case 1:
        model = KisVisualColorModel::HSL;
        break;
    case 2:
        model = KisVisualColorModel::HSI;
        break;
    case 3:
        model = KisVisualColorModel::HSY;
    }
    m_selectorConfigGrid->setColorModel(model);
    m_ui->btnSelectorShape->setIcon(m_selectorConfigGrid->currentIcon());
}


WGColorSelectorSettingsDialog::WGColorSelectorSettingsDialog(QWidget *parent)
    : QDialog(parent)
    , m_widget(new WGColorSelectorSettings(this))
{
    QLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_widget);

    m_widget->loadPreferences();

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::RestoreDefaults,
                                                       Qt::Horizontal,
                                                       this);
    layout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), m_widget, SLOT(savePreferences()));
    connect(buttonBox, SIGNAL(accepted()), this,     SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this,     SLOT(reject()));
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults),
            SIGNAL(clicked()),  m_widget, SLOT(loadDefaultPreferences()));
}
