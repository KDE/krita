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
#include <kis_deformpaintopsettings.h>

#include <KoColorSpaceRegistry.h>

#include <kis_image.h>
#include <kis_debug.h>

#include <kis_paintop_registry.h>
#include <kis_painter.h>
#include <kis_paint_device.h>
#include <kis_paint_information.h>

#include <KoColor.h>
#include <qdebug.h>

class KisDeformOpWidget : public KisConfigWidget
{
public:
    KisDeformOpWidget()
        : KisConfigWidget()
    {
    }

    virtual ~KisDeformOpWidget(){}

    virtual void setConfiguration(KisPropertiesConfiguration * config)
    {
    }

    virtual KisPropertiesConfiguration* configuration() const
    {
        return 0;
    }

};

KisDeformPaintOpSettings::KisDeformPaintOpSettings(QWidget * parent)
        : KisPaintOpSettings()
{
    m_optionsWidget = new KisDeformOpWidget();
    m_options = new Ui::WdgDeformOptions();
    m_options->setupUi(m_optionsWidget);
}

KisPaintOpSettingsSP KisDeformPaintOpSettings::clone() const
{
    KisDeformPaintOpSettings* s = new KisDeformPaintOpSettings(0);
    return s;
}

int KisDeformPaintOpSettings::radius() const
{
    return m_options->deformRadiusSPBox->value();
}


double KisDeformPaintOpSettings::deformAmount() const
{
    return m_options->deformAmountSPBox->value();
}

bool KisDeformPaintOpSettings::bilinear() const
{
    return m_options->interpolationChBox->isChecked();
}

int KisDeformPaintOpSettings::deformAction() const
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
    } else{ 
        return -1; 
    }
}

void KisDeformPaintOpSettings::fromXML(const QDomElement&)
{
    // XXX: save to xml. See for instance the color adjustment filters
}

void KisDeformPaintOpSettings::toXML(QDomDocument&, QDomElement&) const
{
    // XXX: load from xml. See for instance the color adjustment filters
}