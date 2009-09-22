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

#include "kis_sumi_paintop_settings.h"

KisSumiPaintOpSettingsWidget:: KisSumiPaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    m_options = new Ui::WdgSumieOptions();
    m_options->setupUi(this);

}

KisSumiPaintOpSettingsWidget::~ KisSumiPaintOpSettingsWidget()
{
}

void  KisSumiPaintOpSettingsWidget::setConfiguration( const KisPropertiesConfiguration * config)
{
    // XXX: Use a regular curve option here!
    QList<float> c;
    int count = config->getInt( "curve_count" );
    for ( int i = 0; i < count; ++i ) {
        c << config->getFloat( QString("ink_curve_%1").arg( i) );
    }

    m_options->radiusSpinBox->setValue( config->getInt( "radius" ) );
    m_options->sigmaSpinBox->setValue( config->getDouble( "sigma" ) );
    int brushDimensions = config->getInt( "brush_dimension" );
    if ( brushDimensions == 1 ) {
        m_options->oneDimBrushBtn->setChecked( true );
    }
    else {
        m_options->twoDimBrushBtn->setChecked( true );
    }
    m_options->inkAmountSpinBox->setValue( config->getInt( "ink_amount" ) );
    m_options->mousePressureCBox->setChecked( config->getBool( "mouse_pressure" ) );
    m_options->saturationCBox->setChecked( config->getBool( "use_saturation") );
    m_options->opacityCBox->setChecked( config->getBool( "use_opacity" ) );
    m_options->weightSaturationCBox->setChecked( config->getBool( "use_weights" ) );
    m_options->pressureSlider->setValue( config->getInt( "pressure_weight" ) );
    m_options->bristleLengthSlider->setValue( config->getInt( "bristle_length_weight" ) );
    m_options->bristleInkAmountSlider->setValue( config->getInt( "bristle_ink_amount_weight" ) );
    m_options->inkDepletionSlider->setValue( config->getInt( "ink_depletion_weight" ) );
    m_options->shearBox->setValue( config->getDouble( "shear_factor" ) );
    m_options->rndBox->setValue( config->getDouble( "random_factor" ) );
    m_options->scaleBox->setValue( config->getDouble( "scale_factor" ) );
}

KisPropertiesConfiguration*  KisSumiPaintOpSettingsWidget::configuration() const
{
    KisSumiPaintOpSettings* settings = new KisSumiPaintOpSettings();
    settings->setOptionsWidget( const_cast<KisSumiPaintOpSettingsWidget*>( this ) );
//     settings->dump();
    return settings;
}

void KisSumiPaintOpSettingsWidget::writeConfiguration( KisPropertiesConfiguration* config ) const
{
    config->setProperty("paintop", "sumibrush"); // XXX: make this a const id string
    QList<float> c = curve();
    config->setProperty( "curve_count", c.count() );
    for ( int i = 0; i < c.count(); ++i ) {
        config->setProperty( QString("ink_curve_%1").arg( i), c[i] );
    }
    config->setProperty( "radius", radius() );
    config->setProperty( "sigma", sigma() );
    config->setProperty( "brush_dimension", brushDimension() );
    config->setProperty( "ink_amount", inkAmount() );
    config->setProperty( "mouse_pressure", mousePressure() );
    config->setProperty( "use_saturation", useSaturation() );
    config->setProperty( "use_opacity", useOpacity() );
    config->setProperty( "use_weights", useWeights() );
    config->setProperty( "pressure_weight", pressureWeight() );
    config->setProperty( "bristle_length_weight", bristleLengthWeight() );
    config->setProperty( "bristle_ink_amount_weight", bristleInkAmountWeight() );
    config->setProperty( "ink_depletion_weight", inkDepletionWeight() );
    config->setProperty( "shear_factor", shearFactor() );
    config->setProperty( "random_factor", randomFactor() );
    config->setProperty( "scale_factor", scaleFactor() );
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

bool  KisSumiPaintOpSettingsWidget::mousePressure() const
{
    return m_options->mousePressureCBox->isChecked();
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
