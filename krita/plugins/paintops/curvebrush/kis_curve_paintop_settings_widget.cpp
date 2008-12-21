/*
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
#include <kis_curve_paintop_settings_widget.h>

#include <KoColorSpaceRegistry.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>

#include <KoColor.h>

#include "kis_curve_paintop_settings.h"

KisCurvePaintOpSettingsWidget:: KisCurvePaintOpSettingsWidget(QWidget* parent)
    : KisPaintOpSettingsWidget(parent)
{
    m_options = new Ui::WdgCurveOptions();
    m_options->setupUi(this);
}

KisCurvePaintOpSettingsWidget::~ KisCurvePaintOpSettingsWidget()
{
}

void  KisCurvePaintOpSettingsWidget::setConfiguration( const KisPropertiesConfiguration * config)
{
    m_options->curveRadiusSPBox->setValue( config->getInt( "radius" ) );
    m_options->curveAmountSPBox->setValue( config->getDouble( "curve_amount" ) );
    m_options->interpolationChBox->setChecked( config->getBool( "bilinear" ) );
    m_options->addPaintChBox->setChecked( config->getBool( "use_movement_paint" ) );
    m_options->useCounter->setChecked( config->getBool( "use_counter" ) );
    m_options->useOldData->setChecked(config->getBool( "use_old_data" ) );

    int curveAction = config->getInt( "curve_action" );
    if ( curveAction == 1 )
    {
        m_options->growBtn->setChecked( true );
    } else if ( curveAction == 2 ){
        m_options->shrinkBtn->setChecked( true );
    }else if ( curveAction == 3 ){
        m_options->swirlCWBtn->setChecked( true );
    }else if ( curveAction == 4 ){
        m_options->swirlCCWBtn->setChecked( true );
    }else if ( curveAction == 5){
        m_options->moveBtn->setChecked( true );
    }else if ( curveAction == 6 ){
        m_options->lensBtn->setChecked( true );
    }else if ( curveAction == 7 ){
        m_options->lensOutBtn->setChecked( true );
    }else if ( curveAction == 8 ){
        m_options->colorBtn->setChecked( true );
    }
}

KisPropertiesConfiguration*  KisCurvePaintOpSettingsWidget::configuration() const
{
    KisCurvePaintOpSettings* settings = new KisCurvePaintOpSettings( const_cast<KisCurvePaintOpSettingsWidget*>( this ) );
    return settings;
}

void KisCurvePaintOpSettingsWidget::writeConfiguration( KisPropertiesConfiguration* config ) const
{
    config->setProperty( "radius", radius() );
    config->setProperty( "curve_amount", curveAmount() );
    config->setProperty( "curve_action", curveAction() );
    config->setProperty( "bilinear", bilinear() );
    config->setProperty( "use_movement_paint", useMovementPaint() );
    config->setProperty( "use_counter", useCounter() );
    config->setProperty( "use_old_data", useOldData() );
}

int  KisCurvePaintOpSettingsWidget::radius() const
{
    return m_options->curveRadiusSPBox->value();
}

double  KisCurvePaintOpSettingsWidget::curveAmount() const
{
    return m_options->curveAmountSPBox->value();
}

int  KisCurvePaintOpSettingsWidget::curveAction() const
{
    //TODO: make it nicer using enums or something
    if ( m_options->growBtn->isChecked() )
    {
        return 1;
    } else if ( m_options->shrinkBtn->isChecked() ){
        return 2;
    }else if ( m_options->swirlCWBtn->isChecked() ){
        return 3;
    }else if ( m_options->swirlCCWBtn->isChecked() ){
        return 4;
    } else if ( m_options->moveBtn->isChecked() ){ 
        return 5; 
    } else if ( m_options->lensBtn->isChecked() ){ 
        return 6; 
    } else if ( m_options->lensOutBtn->isChecked() ){ 
        return 7; 
    } else if ( m_options->colorBtn->isChecked() ){ 
        return 8; 
    } else{
        return -1;
    }
}

bool  KisCurvePaintOpSettingsWidget::bilinear() const
{
    return m_options->interpolationChBox->isChecked();
}

bool KisCurvePaintOpSettingsWidget::useMovementPaint() const{
    return m_options->addPaintChBox->isChecked();
}

bool KisCurvePaintOpSettingsWidget::useCounter() const{
    return m_options->useCounter->isChecked();
}

bool KisCurvePaintOpSettingsWidget::useOldData() const{
    return m_options->useOldData->isChecked();
}

