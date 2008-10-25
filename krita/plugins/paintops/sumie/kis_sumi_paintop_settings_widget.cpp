/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <kis_sumi_paintop_settings_widget.h>

#include <KoColorSpaceRegistry.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>

#include <KoColor.h>
#include <qdebug.h>

KisSumiPaintOpSettingsWidget:: KisSumiPaintOpSettingsWidget()
    : KisPaintOpSettingsWidget()
{
    m_options = new Ui::WdgSumieOptions();
    m_options->setupUi(this);

}

KisSumiPaintOpSettingsWidget::~ KisSumiPaintOpSettingsWidget(){}

void  KisSumiPaintOpSettingsWidget::setConfiguration(KisPropertiesConfiguration * config)
{
    // XXX
}

KisPropertiesConfiguration*  KisSumiPaintOpSettingsWidget::configuration() const
{
    // XXX
    return 0;
}

void KisSumiPaintOpSettingsWidget::writeConfiguration( KisPropertiesConfiguration* config ) const
{
    // XXX
}


QList<float> KisSumiPaintOpSettingsWidget::curve() const
{
    int curveSamples = inkAmount();
    QList<float> result;
    for (int i = 0; i < curveSamples ; i++) {
        result.append((float)m_options->inkCurve->getCurveValue(i / (float)(curveSamples - 1.0f)));
    }
    return result;
}

int  KisSumiPaintOpSettingsWidget::radius() const
{
    return m_options->radiusSpinBox->value();
}

double  KisSumiPaintOpSettingsWidget::sigma() const
{
    return m_options->sigmaSpinBox->value();
}

bool  KisSumiPaintOpSettingsWidget::mousePressure() const
{
    return m_options->mousePressureCBox->isChecked();
}

int  KisSumiPaintOpSettingsWidget::brushDimension() const
{
    if (m_options->oneDimBrushBtn->isChecked()) {
        return 1;
    } else
        return 2;
}

int  KisSumiPaintOpSettingsWidget::inkAmount() const
{
    return m_options->inkAmountSpinBox->value();
}

double  KisSumiPaintOpSettingsWidget::shearFactor() const
{
    return m_options->shearBox->value();
}

double  KisSumiPaintOpSettingsWidget::randomFactor() const
{
    return m_options->rndBox->value();
}

double  KisSumiPaintOpSettingsWidget::scaleFactor() const
{
    return m_options->scaleBox->value();
}

bool  KisSumiPaintOpSettingsWidget::useSaturation() const
{
    return m_options->saturationCBox->isChecked();
}

bool  KisSumiPaintOpSettingsWidget::useOpacity() const
{
    return m_options->opacityCBox->isChecked();
}

bool  KisSumiPaintOpSettingsWidget::useWeights() const
{
    return m_options->weightSaturationCBox->isChecked();
}

int  KisSumiPaintOpSettingsWidget::pressureWeight() const
{
    return m_options->pressureSlider->value();
}

int  KisSumiPaintOpSettingsWidget::bristleLengthWeight() const
{
    return m_options->bristleLengthSlider->value();
}

int  KisSumiPaintOpSettingsWidget::bristleInkAmountWeight() const
{
    return m_options->bristleInkAmountSlider->value();
}

int  KisSumiPaintOpSettingsWidget::inkDepletionWeight() const
{
    return m_options->inkDepletionSlider->value();
}

