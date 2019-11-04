/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_sketchop_option.h"

#include "ui_wdgsketchoptions.h"
#include <brushengine/kis_paintop_lod_limitations.h>


class KisSketchOpOptionsWidget: public QWidget, public Ui::WdgSketchOptions
{
public:
    KisSketchOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KisSketchOpOption::KisSketchOpOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisSketchOpOption");

    m_checkable = false;
    m_options = new KisSketchOpOptionsWidget();

    // initialize slider values
    m_options->lineWidthSPBox->setRange(1.0, 100.0, 0);
    m_options->lineWidthSPBox->setValue(1.0);
    m_options->lineWidthSPBox->setSuffix(i18n(" px"));
    m_options->lineWidthSPBox->setExponentRatio(1.5);


    m_options->offsetSPBox->setRange(0.0, 200.0, 0);
    m_options->offsetSPBox->setValue(30.0);
    m_options->offsetSPBox->setSuffix(i18n("%"));

    m_options->densitySPBox->setRange(0.0, 100.0, 0);
    m_options->densitySPBox->setValue(50.0);
    m_options->densitySPBox->setSuffix(i18n("%"));


    connect(m_options->offsetSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->lineWidthSPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->densitySPBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->simpleModeCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->connectionCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->magnetifyCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->randomRGBCHbox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->randomOpacityCHbox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->distanceDensityCHBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->distanceOpacityCHbox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));


    setConfigurationPage(m_options);
}

KisSketchOpOption::~KisSketchOpOption()
{
    // delete m_options;
}

void KisSketchOpOption::writeOptionSetting(KisPropertiesConfigurationSP settings) const
{
    settings->setProperty(SKETCH_OFFSET, m_options->offsetSPBox->value());
    settings->setProperty(SKETCH_PROBABILITY, m_options->densitySPBox->value() * 0.01);
    settings->setProperty(SKETCH_USE_SIMPLE_MODE, m_options->simpleModeCHBox->isChecked());
    settings->setProperty(SKETCH_MAKE_CONNECTION, m_options->connectionCHBox->isChecked());
    settings->setProperty(SKETCH_MAGNETIFY, m_options->magnetifyCHBox->isChecked());
    settings->setProperty(SKETCH_RANDOM_RGB, m_options->randomRGBCHbox->isChecked());
    settings->setProperty(SKETCH_LINE_WIDTH, m_options->lineWidthSPBox->value());
    settings->setProperty(SKETCH_RANDOM_OPACITY, m_options->randomOpacityCHbox->isChecked());
    settings->setProperty(SKETCH_DISTANCE_DENSITY, m_options->distanceDensityCHBox->isChecked());
    settings->setProperty(SKETCH_DISTANCE_OPACITY, m_options->distanceOpacityCHbox->isChecked());
}

void KisSketchOpOption::readOptionSetting(const KisPropertiesConfigurationSP settings)
{
    m_options->offsetSPBox->setValue(settings->getDouble(SKETCH_OFFSET));
    m_options->simpleModeCHBox->setChecked(settings->getBool(SKETCH_USE_SIMPLE_MODE));
    m_options->connectionCHBox->setChecked(settings->getBool(SKETCH_MAKE_CONNECTION));
    m_options->magnetifyCHBox->setChecked(settings->getBool(SKETCH_MAGNETIFY));
    m_options->lineWidthSPBox->setValue(settings->getInt(SKETCH_LINE_WIDTH));
    m_options->densitySPBox->setValue(settings->getDouble(SKETCH_PROBABILITY) * 100.0);
    m_options->randomRGBCHbox->setChecked(settings->getBool(SKETCH_RANDOM_RGB));
    m_options->randomOpacityCHbox->setChecked(settings->getBool(SKETCH_RANDOM_OPACITY));
    m_options->distanceDensityCHBox->setChecked(settings->getBool(SKETCH_DISTANCE_DENSITY));
    m_options->distanceOpacityCHbox->setChecked(settings->getBool(SKETCH_DISTANCE_OPACITY));
}

void KisSketchOpOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    l->limitations << KoID("sketch-brush", i18nc("PaintOp instant preview limitation", "Sketch brush (differences in connecting lines are possible)"));
}
