/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_airbrush_option.h"
#include "kis_paintop_settings.h"
#include <klocalizedstring.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgairbrush.h"

const qreal MINIMUM_RATE = 1.0;
const qreal MAXIMUM_RATE = 1000.0;
const int RATE_NUM_DECIMALS = 2;
const qreal RATE_EXPONENT_RATIO = 2.0;
const qreal RATE_SINGLE_STEP = 1.0;
const qreal DEFAULT_RATE = 20.0;

class KisAirbrushWidget: public QWidget, public Ui::WdgAirbrush
{
public:
    KisAirbrushWidget(QWidget *parent = 0, bool canIgnoreSpacing = true)
        : QWidget(parent) {
        setupUi(this);

        sliderRate->setRange(MINIMUM_RATE, MAXIMUM_RATE, RATE_NUM_DECIMALS);
        sliderRate->setExponentRatio(RATE_EXPONENT_RATIO);
        sliderRate->setSingleStep(RATE_SINGLE_STEP);
        sliderRate->setValue(DEFAULT_RATE);

        checkBoxIgnoreSpacing->setVisible(canIgnoreSpacing);
        checkBoxIgnoreSpacing->setEnabled(canIgnoreSpacing);
    }
};

struct KisAirbrushOption::Private {
public:
    bool ignoreSpacing;
    // We store the airbrush interval (in milliseconds) instead of the rate because the interval is
    // likely to be accessed more often.
    qreal airbrushInterval;
    KisAirbrushWidget *configPage;
};

KisAirbrushOption::KisAirbrushOption(bool enabled, bool canIgnoreSpacing)
    : KisPaintOpOption(KisPaintOpOption::COLOR, enabled)
    , m_d(new Private())
{
    setObjectName("KisAirbrushOption");

    // Initialize GUI.
    m_checkable = true;
    m_d->configPage = new KisAirbrushWidget(nullptr, canIgnoreSpacing);
    connect(m_d->configPage->sliderRate, SIGNAL(valueChanged(qreal)), SLOT(slotIntervalChanged()));
    connect(m_d->configPage->checkBoxIgnoreSpacing, SIGNAL(toggled(bool)),
            SLOT(slotIgnoreSpacingChanged()));
    setConfigurationPage(m_d->configPage);

    // Read initial configuration from the GUI.
    updateIgnoreSpacing();
    updateInterval();
}

KisAirbrushOption::~KisAirbrushOption()
{
    delete m_d;
}

void KisAirbrushOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KIS_SAFE_ASSERT_RECOVER (m_d->airbrushInterval > 0.0) {
        m_d->airbrushInterval = 1.0;
    }
    setting->setProperty(AIRBRUSH_ENABLED, isChecked());
    setting->setProperty(AIRBRUSH_RATE, 1000.0 / m_d->airbrushInterval);
    setting->setProperty(AIRBRUSH_IGNORE_SPACING, m_d->ignoreSpacing);
}

void KisAirbrushOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    setChecked(setting->getBool(AIRBRUSH_ENABLED));
    // Update settings in the widget. The widget's signals should cause the changes to be propagated
    // to this->m_d as well.
    m_d->configPage->sliderRate->setValue(setting->getDouble(AIRBRUSH_RATE, DEFAULT_RATE));
    m_d->configPage->checkBoxIgnoreSpacing->setChecked(setting->getBool(AIRBRUSH_IGNORE_SPACING,
                                                                        false));
}

qreal KisAirbrushOption::airbrushInterval() const
{
    return m_d->airbrushInterval;
}

bool KisAirbrushOption::ignoreSpacing() const
{
    return m_d->ignoreSpacing;
}

void KisAirbrushOption::slotIntervalChanged()
{
    updateInterval();
    emitSettingChanged();
}

void KisAirbrushOption::slotIgnoreSpacingChanged()
{
    updateIgnoreSpacing();
    emitSettingChanged();
}

void KisAirbrushOption::updateInterval()
{
    // Get rate in dabs per second, then convert to interval in milliseconds.
    qreal rate = m_d->configPage->sliderRate->value();
    KIS_SAFE_ASSERT_RECOVER(rate > 0.0) {
        rate = 1.0;
    }
    m_d->airbrushInterval = 1000.0 / rate;
}

void KisAirbrushOption::updateIgnoreSpacing()
{
    m_d->ignoreSpacing = m_d->configPage->checkBoxIgnoreSpacing->isChecked();
}
