/*
 *  Copyright (c) 2008-2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_hairy_bristle_option.h"
#include <klocalizedstring.h>

#include "ui_wdgbristleoptions.h"
#include <brushengine/kis_paintop_lod_limitations.h>


class KisBristleOptionsWidget: public QWidget, public Ui::WdgBristleOptions
{
public:
    KisBristleOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);

        scaleBox->setPrefix(i18n("Random Offset: "));
        rndBox->setRange(-10.0, 10.0, 2);
        rndBox->setValue(2.0);

        scaleBox->setPrefix(i18n("Scale: "));
        scaleBox->setRange(-10.0, 10.0, 2);
        scaleBox->setValue(2.0);

        shearBox->setPrefix(i18n("Shear: "));
        shearBox->setRange(-2.0, 2.0, 2);
        shearBox->setValue(0.0);

        densityBox->setPrefix(i18n("Density: "));
        densityBox->setRange(0.0, 100.0, 0);
        densityBox->setValue(100.0);
        densityBox->setSuffix(QChar(Qt::Key_Percent));
    }
};

KisHairyBristleOption::KisHairyBristleOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisHairyBristleOption");


    m_checkable = false;
    m_options = new KisBristleOptionsWidget();

    // signals
    connect(m_options->mousePressureCBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->thresholdCBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->rndBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->scaleBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->shearBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->densityBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->connectedCBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->antialiasCBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->compositingCBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    setConfigurationPage(m_options);
}

KisHairyBristleOption::~KisHairyBristleOption()
{
    delete m_options;
}

void KisHairyBristleOption::readOptionSetting(const KisPropertiesConfigurationSP config)
{
    m_options->thresholdCBox->setChecked(config->getBool(HAIRY_BRISTLE_THRESHOLD));
    m_options->mousePressureCBox->setChecked(config->getBool(HAIRY_BRISTLE_USE_MOUSEPRESSURE));
    m_options->shearBox->setValue(config->getDouble(HAIRY_BRISTLE_SHEAR));
    m_options->rndBox->setValue(config->getDouble(HAIRY_BRISTLE_RANDOM));
    m_options->scaleBox->setValue(config->getDouble(HAIRY_BRISTLE_SCALE));
    m_options->densityBox->setValue(config->getDouble(HAIRY_BRISTLE_DENSITY));
    m_options->connectedCBox->setChecked(config->getBool(HAIRY_BRISTLE_CONNECTED));
    m_options->antialiasCBox->setChecked(config->getBool(HAIRY_BRISTLE_ANTI_ALIASING));
    m_options->compositingCBox->setChecked(config->getBool(HAIRY_BRISTLE_USE_COMPOSITING));
}


void KisHairyBristleOption::writeOptionSetting(KisPropertiesConfigurationSP config) const
{
    config->setProperty(HAIRY_BRISTLE_THRESHOLD, m_options->thresholdCBox->isChecked());
    config->setProperty(HAIRY_BRISTLE_USE_MOUSEPRESSURE, m_options->mousePressureCBox->isChecked());
    config->setProperty(HAIRY_BRISTLE_SCALE, m_options->scaleBox->value());
    config->setProperty(HAIRY_BRISTLE_SHEAR, m_options->shearBox->value());
    config->setProperty(HAIRY_BRISTLE_RANDOM, m_options->rndBox->value());
    config->setProperty(HAIRY_BRISTLE_DENSITY, m_options->densityBox->value());
    config->setProperty(HAIRY_BRISTLE_CONNECTED, m_options->connectedCBox->isChecked());
    config->setProperty(HAIRY_BRISTLE_ANTI_ALIASING, m_options->antialiasCBox->isChecked());
    config->setProperty(HAIRY_BRISTLE_USE_COMPOSITING, m_options->compositingCBox->isChecked());
}

void KisHairyBristleOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    l->limitations << KoID("hairy-brush", i18nc("PaintOp instant preview limitation", "Bristle Brush (the lines will be thinner than on preview)"));
}
