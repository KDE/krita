/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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
#include "kis_roundmarker_option.h"

#include "kis_signals_blocker.h"

#include <kconfig.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "ui_kis_roundmarker_option.h"


class KisRoundMarkerOptionWidget: public QWidget, public Ui::WdgKisRoundMarkerOption
{
public:
    KisRoundMarkerOptionWidget(QWidget *parent = 0) : QWidget(parent) {
        setupUi(this);

        const int maxBrushSize = KSharedConfig::openConfig()->group("").readEntry("maximumBrushSize", 1000);

        dblDiameter->setRange(0.01, maxBrushSize, 2);
        dblDiameter->setPrefix(i18n("Diameter: "));
        dblDiameter->setSuffix(i18n(" px"));
    }
};

KisRoundMarkerOption::KisRoundMarkerOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    m_checkable = false;
    m_options = new KisRoundMarkerOptionWidget();

    connect(m_options->spacingWidget, SIGNAL(sigSpacingChanged()), this, SLOT(emitSettingChanged()));
    connect(m_options->dblDiameter, SIGNAL(valueChanged(qreal)), this, SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);

    setObjectName("KisRoundMarkerOption");
}

KisRoundMarkerOption::~KisRoundMarkerOption()
{
}

void KisRoundMarkerOption::writeOptionSetting(KisPropertiesConfigurationSP config) const
{
    RoundMarkerOption op;

    op.diameter = m_options->dblDiameter->value();
    op.spacing = m_options->spacingWidget->spacing();
    op.use_auto_spacing = m_options->spacingWidget->autoSpacingActive();
    op.auto_spacing_coeff = m_options->spacingWidget->autoSpacingCoeff();

    op.writeOptionSetting(config);
}

void KisRoundMarkerOption::readOptionSetting(KisPropertiesConfigurationSP config)
{
    RoundMarkerOption op;
    op.readOptionSetting(*config);

    KisSignalsBlocker b(m_options->dblDiameter, m_options->spacingWidget);

    m_options->dblDiameter->setValue(op.diameter);
    m_options->spacingWidget->setSpacing(op.use_auto_spacing,
                                         op.use_auto_spacing ?
                                         op.auto_spacing_coeff : op.spacing);
}


