/* This file is part of the KDE project
 *
 * Copyright (C) 2015 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#include "kis_tangent_tilt_option.h"
#include <cmath>
#include <QColor>
#include <QPoint>

#include "ui_wdgtangenttiltoption.h"

#include "kis_global.h"

class KisTangentTiltOptionWidget: public QWidget, public Ui::WdgTangentTiltOptions
{
public:
    KisTangentTiltOptionWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KisTangentTiltOption::KisTangentTiltOption()
: KisPaintOpOption(KisPaintOpOption::GENERAL, false),
            m_canvasAngle(0.0),
            m_canvasAxisXMirrored(false),
            m_canvasAxisYMirrored(false)
{
    m_checkable = false;
    m_options = new KisTangentTiltOptionWidget();
    //Setup tangent tilt.
    m_options->comboRed->setCurrentIndex(0);
    m_options->comboGreen->setCurrentIndex(2);
    m_options->comboBlue->setCurrentIndex(4);

    m_options->sliderElevationSensitivity->setRange(0, 100, 0);
    m_options->sliderElevationSensitivity->setValue(100);
    m_options->sliderElevationSensitivity->setSuffix("%");

    m_options->sliderMixValue->setRange(0, 100, 0);
    m_options->sliderMixValue->setValue(50);
    m_options->sliderMixValue->setSuffix("%");

    connect(m_options->comboRed, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->comboGreen, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->comboBlue, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->comboRed, SIGNAL(currentIndexChanged(int)), m_options->TangentTiltPreview, SLOT(setRedChannel(int)));
    connect(m_options->comboGreen, SIGNAL(currentIndexChanged(int)), m_options->TangentTiltPreview, SLOT(setGreenChannel(int)));
    connect(m_options->comboBlue, SIGNAL(currentIndexChanged(int)), m_options->TangentTiltPreview, SLOT(setBlueChannel(int)));

    connect(m_options->optionTilt, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->optionDirection, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->optionRotation, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->optionMix, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));

    connect(m_options->sliderElevationSensitivity, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_options->sliderMixValue, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    m_options->sliderMixValue->setVisible(false);

    setConfigurationPage(m_options);

}

KisTangentTiltOption::~KisTangentTiltOption()
{
    delete m_options;
}

//options
int KisTangentTiltOption::redChannel() const
{
    return m_options->comboRed->currentIndex();
}
int KisTangentTiltOption::greenChannel() const
{
    return m_options->comboGreen->currentIndex();
}
int KisTangentTiltOption::blueChannel() const
{
    return m_options->comboBlue->currentIndex();
}

int KisTangentTiltOption::directionType() const
{
    int type=0;

    if (m_options->optionTilt->isChecked()==true) {
        type=0;
    }
    else if (m_options->optionDirection->isChecked()==true) {
        type=1;
    }
    else if (m_options->optionRotation->isChecked()==true) {
        type=2;
    }
    else if (m_options->optionMix->isChecked()==true) {
        type=3;
    }
    else {
        warnKrita<<"There's something odd with the radio buttons. We'll use Tilt";
    }

    return type;
}

double KisTangentTiltOption::elevationSensitivity() const
{
    return m_options->sliderElevationSensitivity->value(); 
}

double KisTangentTiltOption::mixValue() const
{
    return m_options->sliderMixValue->value();
}

void KisTangentTiltOption::swizzleAssign(qreal const horizontal, qreal const vertical, qreal const depth, qreal *component, int index, qreal maxvalue)
{
    switch(index) {
    case 0: *component = horizontal; break;
    case 1: *component = maxvalue-horizontal; break;
    case 2: *component = vertical; break;
    case 3: *component = maxvalue-vertical; break;
    case 4: *component = depth; break;
    case 5: *component = maxvalue-depth; break;
    }
}

void KisTangentTiltOption::apply(const KisPaintInformation& info,qreal *r,qreal *g,qreal *b)
{
    //formula based on http://www.cerebralmeltdown.com/programming_projects/Altitude%20and%20Azimuth%20to%20Vector/index.html

    /* It doesn't make sense of have higher than 8bit color depth.
     * Instead we make sure in the paintAt function of kis_tangent_normal_paintop to pick an 8bit space of the current
     * color space if the space is an RGB space. If not, it'll pick sRGB 8bit.
     */
    qreal halfvalue = 0.5;
    qreal maxvalue = 1.0;

    //have the azimuth and altitude in degrees.
    qreal direction = KisPaintInformation::tiltDirection(info, true)*360.0;
    qreal elevation= (info.tiltElevation(info, 60.0, 60.0, true)*90.0);
    if (directionType()==0) {
        direction = KisPaintInformation::tiltDirection(info, true)*360.0;
	elevation= (info.tiltElevation(info, 60.0, 60.0, true)*90.0);
    } else if (directionType()==1) {
        direction = (0.75 + info.drawingAngle() / (2.0 * M_PI))*360.0;
	elevation= 0;//turns out that tablets that don't support tilt just return 90 degrees for elevation.
    } else if (directionType()==2) {
        direction = info.rotation();
	elevation= (info.tiltElevation(info, 60.0, 60.0, true)*90.0);//artpens have tilt-recognition, so this should work.
    } else if (directionType()==3) {//mix of tilt+direction
	qreal mixamount = mixValue()/100.0;
        direction = (KisPaintInformation::tiltDirection(info, true)*360.0*(1.0-mixamount))+((0.75 + info.drawingAngle() / (2.0 * M_PI))*360.0*(mixamount));
	elevation= (info.tiltElevation(info, 60.0, 60.0, true)*90.0);
    }

    //subtract/add the rotation of the canvas.

    if (info.canvasRotation()!=m_canvasAngle && info.canvasMirroredH()==m_canvasAxisXMirrored) {
       m_canvasAngle=info.canvasRotation();
    }
    if (directionType()!=1) {
        direction = direction-m_canvasAngle;
    }

    //limit the direction/elevation

    //qreal elevationMax = (elevationSensitivity()*90.0)/100.0;
    qreal elevationT = elevation*(elevationSensitivity()/100.0)+(90-(elevationSensitivity()*90.0)/100.0);
    elevation = static_cast<int>(elevationT);

    //convert to radians.
    // Convert this to kis_global's radian function.
    direction = kisDegreesToRadians(direction);
    elevation = kisDegreesToRadians(elevation);

    //make variables for axes for easy switching later on.
    qreal horizontal, vertical, depth;

    //spherical coordinates always center themselves around the origin, leading to values. We need to work around those...

    horizontal = cos(elevation)*sin(direction);
    if (horizontal>0.0) {
        horizontal= halfvalue+(fabs(horizontal)*halfvalue);
    }
    else {
        horizontal= halfvalue-(fabs(horizontal)*halfvalue);
    }
    vertical = cos(elevation)*cos(direction);
    if (vertical>0.0) {
        vertical = halfvalue+(fabs(vertical)*halfvalue);
    }
    else {
        vertical = halfvalue-(fabs(vertical)*halfvalue);
    }

    if (m_canvasAxisXMirrored && info.canvasMirroredH()) {horizontal = maxvalue-horizontal;}
    if (m_canvasAxisYMirrored && info.canvasMirroredH()) {vertical = maxvalue-vertical;}

    depth = sin(elevation)*maxvalue;

    //assign right components to correct axes.
    swizzleAssign(horizontal, vertical, depth, r, redChannel(), maxvalue);
    swizzleAssign(horizontal, vertical, depth, g, greenChannel(), maxvalue);
    swizzleAssign(horizontal, vertical, depth, b, blueChannel(), maxvalue);

}

/*settings*/
void KisTangentTiltOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    setting->setProperty(TANGENT_RED, redChannel());
    setting->setProperty(TANGENT_GREEN, greenChannel());
    setting->setProperty(TANGENT_BLUE, blueChannel());
    setting->setProperty(TANGENT_TYPE, directionType());
    setting->setProperty(TANGENT_EV_SEN, elevationSensitivity());
    setting->setProperty(TANGENT_MIX_VAL, mixValue());
}

void KisTangentTiltOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    m_options->comboRed->setCurrentIndex(setting->getInt(TANGENT_RED, 0));
    m_options->comboGreen->setCurrentIndex(setting->getInt(TANGENT_GREEN, 2));
    m_options->comboBlue->setCurrentIndex(setting->getInt(TANGENT_BLUE, 4));
    
    //The comboboxes are connected to the TangentTiltPreview, so that gets automatically updated by them.

    if (setting->getInt(TANGENT_TYPE)== 0){
        m_options->optionTilt->setChecked(true);
	m_options->sliderMixValue->setVisible(false);
    }
    else if (setting->getInt(TANGENT_TYPE)== 1) {
        m_options->optionDirection->setChecked(true);
	m_options->sliderMixValue->setVisible(false);
    }
    else if (setting->getInt(TANGENT_TYPE)== 2) {
        m_options->optionRotation->setChecked(true);
	m_options->sliderMixValue->setVisible(false);
    }
    else if (setting->getInt(TANGENT_TYPE)== 3) {
        m_options->optionMix->setChecked(true);
	m_options->sliderMixValue->setVisible(true);
    }

    m_canvasAngle = setting->getDouble("runtimeCanvasRotation", 0.0);//in degrees please.
    m_canvasAxisXMirrored = setting->getBool("runtimeCanvasMirroredX", false);
    m_canvasAxisYMirrored = setting->getBool("runtimeCanvasMirroredY", false);

    m_options->sliderElevationSensitivity->setValue(setting->getDouble(TANGENT_EV_SEN, 100));
    m_options->sliderMixValue->setValue(setting->getDouble(TANGENT_MIX_VAL, 50));

}
