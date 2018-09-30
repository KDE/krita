/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include "kis_brush_size_option.h"
#include <klocalizedstring.h>

#include <QWidget>
#include <QRadioButton>

#include "ui_wdgBrushSizeOptions.h"

class KisBrushSizeOptionsWidget: public QWidget, public Ui::WdgBrushSizeOptions
{
public:
    KisBrushSizeOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }

};


KisBrushSizeOption::KisBrushSizeOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, false)
{
    setObjectName("KisBrushSizeOption");

    m_checkable = false;
    m_options = new KisBrushSizeOptionsWidget();


    // init slider values
    m_options->diameter->setPrefix(i18n("Diameter: "));
    m_options->diameter->setRange(1.0, 1000, 0);
    m_options->diameter->setValue(20);
    m_options->diameter->setExponentRatio(3.0);
    m_options->diameter->setSuffix(i18n(" px"));

    m_options->aspectBox->setPrefix(i18n("Aspect Ratio: "));
    m_options->aspectBox->setRange(0.01, 2.0, 2);
    m_options->aspectBox->setValue(1.0);
    m_options->aspectBox->setExponentRatio(1.0);

    m_options->scale->setPrefix(i18n("Scale: "));
    m_options->scale->setRange(0.01, 10.0, 2);
    m_options->scale->setValue(1.0);

    m_options->spacing->setPrefix(i18n("Spacing: "));
    m_options->spacing->setRange(0.01, 5.0, 2);
    m_options->spacing->setValue(0.3);

    m_options->rotationBox->setPrefix(i18n("Rotation: "));
    m_options->rotationBox->setRange(0.0, 360.0, 0);
    m_options->rotationBox->setValue(0.0);
    m_options->rotationBox->setSuffix(QChar(Qt::Key_degree));

    m_options->densityBox->setPrefix(i18n("Density: "));
    m_options->densityBox->setRange(0.0, 100.0, 0);
    m_options->densityBox->setValue(100);
    m_options->densityBox->setSuffix("%");

    m_options->jitterMove->setPrefix(i18n("Jitter Move: "));
    m_options->jitterMove->setRange(0.0, 5.0, 2);
    m_options->jitterMove->setValue(0.0);

    connect(m_options->diameter, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->scale, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->aspectBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->spacing, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->rotationBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->densityBox, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->jitterMove, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    connect(m_options->jitterMoveBox, SIGNAL(toggled(bool)), m_options->jitterMove, SLOT(setEnabled(bool)));
    setConfigurationPage(m_options);
}

KisBrushSizeOption::~KisBrushSizeOption()
{
    delete m_options;
}


int KisBrushSizeOption::diameter() const
{
    return qRound(m_options->diameter->value());
}


void KisBrushSizeOption::setDiameter(int diameter)
{
    m_options->diameter->setValue(diameter);
}

qreal KisBrushSizeOption::brushAspect() const
{
    return m_options->aspectBox->value();
}


void KisBrushSizeOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisBrushSizeOptionProperties op;

    op.brush_diameter = m_options->diameter->value();
    op.brush_aspect = m_options->aspectBox->value();
    op.brush_rotation = m_options->rotationBox->value();
    op.brush_scale = m_options->scale->value();
    op.brush_spacing = m_options->spacing->value();
    op.brush_density = m_options->densityBox->value() / 100.0;
    op.brush_jitter_movement = m_options->jitterMove->value();
    op.brush_jitter_movement_enabled = m_options->jitterMoveBox->isChecked();

    op.writeOptionSetting(setting);
}

void KisBrushSizeOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisBrushSizeOptionProperties op;
    op.readOptionSetting(setting);

    m_options->diameter->setValue(op.brush_diameter);
    m_options->aspectBox->setValue(op.brush_aspect);
    m_options->rotationBox->setValue(op.brush_rotation);
    m_options->scale->setValue(op.brush_scale);
    m_options->spacing->setValue(op.brush_spacing);
    m_options->densityBox->setValue(op.brush_density * 100.0);
    m_options->jitterMove->setValue(op.brush_jitter_movement);
    m_options->jitterMoveBox->setChecked(op.brush_jitter_movement_enabled);

}

void KisBrushSizeOption::setSpacing(qreal spacing)
{
    m_options->spacing->setValue(spacing);
}

qreal KisBrushSizeOption::spacing() const
{
    return m_options->spacing->value();
}

