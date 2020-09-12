/*
 * SPDX-FileCopyrightText: 2020 Mathias Wein <lynx.mw+kde@gmail.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "WGColorSelectorSettings.h"
#include "ui_WdgWGSelectorSettings.h"

#include "WGSelectorConfigGrid.h"

#include "kis_config.h"

#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

WGColorSelectorSettings::WGColorSelectorSettings(QWidget *parent)
    : KisPreferenceSet(parent)
    , m_ui(new Ui::WGConfigWidget)
{
    m_ui->setupUi(this);
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
    KConfigGroup cfg = KSharedConfig::openConfig()->group(stringID());
    cfg.writeEntry("renderMode", m_ui->cmbSelectorRenderingMode->currentIndex());
}

void WGColorSelectorSettings::loadPreferences()
{
    KConfigGroup cfg = KSharedConfig::openConfig()->group(stringID());
    m_ui->cmbSelectorRenderingMode->setCurrentIndex(cfg.readEntry("renderMode", 1));
}

void WGColorSelectorSettings::loadDefaultPreferences()
{

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
