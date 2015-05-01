/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Wolthera van Hövell <griffinvalley@gmail.com>
 *  Copyright (c) 2015 Moritz Molch <kde@moritzmolch.de>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_slider_input.h"

#include <KoConfig.h>
#ifdef HAVE_OPENEXR
#include <half.h>
#endif

#include <cmath>

#include <kis_debug.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QFontMetrics>

#include <klocale.h>
#include <kconfiggroup.h>

#include <kglobal.h>
#include <QDoubleSpinBox>


#include <KoColor.h>
#include <KoColorSpace.h>

#include "kis_hsv_slider.h"
#include "kis_display_color_converter.h"

KisColorSliderInput::KisColorSliderInput(QWidget* parent, KoColor* color, const int type, KoColorDisplayRendererInterface *displayRenderer, KisCanvas2* canvas)
    : QWidget(parent),
    m_type(type),
    m_color(color),
    m_displayRenderer(displayRenderer),
    m_canvas(canvas)
{
    //init
}

void KisColorSliderInput::init()
{
    QHBoxLayout* m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(1);

    QString m_name;
    switch (m_type){
    case 0: m_name=i18n("Hue"); break;
    case 1: m_name=i18n("Saturation"); break;
    case 2: m_name=i18n("Value"); break;
    case 3: m_name=i18n("Hue"); break;
    case 4: m_name=i18n("Saturation"); break;
    case 5: m_name=i18n("Lightness"); break;
    case 6: m_name=i18n("Hue"); break;
    case 7: m_name=i18n("Saturation"); break;
    case 8: m_name=i18n("Intensity"); break;
    case 9: m_name=i18n("Hue"); break;
    case 10: m_name=i18n("Saturation"); break;
    case 11: m_name=i18n("Luma"); break;
    }
    
    QLabel* m_label = new QLabel(i18n("%1:", m_name), this);
    m_layout->addWidget(m_label);

    m_hsvSlider = new KisHSVSlider(Qt::Horizontal, this, m_displayRenderer, m_canvas);
    m_hsvSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_layout->addWidget(m_hsvSlider);
    connect (m_hsvSlider,  SIGNAL(sliderPressed()), SLOT(sliderIn()));
    connect (m_hsvSlider,  SIGNAL(sliderReleased()), SLOT(sliderOut()));

    QWidget* m_input = createInput();
    m_hsvSlider->setFixedHeight(m_input->sizeHint().height());
    m_layout->addWidget(m_input);
}

KisHSXColorSliderInput::KisHSXColorSliderInput(QWidget* parent, const int type, KoColor* color, KoColorDisplayRendererInterface *displayRenderer, KisCanvas2* canvas) : KisColorSliderInput(parent, color, type, displayRenderer, canvas),
    m_canvas(canvas),
    m_hue(0),
    m_sat(0),
    m_val(0),
    R(0),
    G(0),
    B(0)
{
    m_hueupdating = false;
    m_satupdating = false;
    m_toneupdating = false;
    m_sliderisupdating = false;
    init();
}

void KisHSXColorSliderInput::setValue(double v)
{

    //This function returns the colour based on the type of the slider as well as the value//

    qreal h=0.0;
    qreal s=0.0;
    qreal l=0.0;
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
    R = cfg.readEntry("lumaR", 0.2126);
    G = cfg.readEntry("lumaG", 0.7152);
    B = cfg.readEntry("lumaB", 0.0722);

    switch (m_type) {
    case 0:
        m_hue = v;
        h=m_hue/360.0;
        s=m_sat/100.0;
        l=m_val/100.0;
        *m_color = this->converter()->fromHsvF(h, s, l);
        if (m_hueupdating==false) {
            emit(hueUpdated(static_cast<int>(m_hue)));
        }
        else {
            m_hueupdating=false;
        }
        break;
    case 3:
        m_hue = v;
        h=m_hue/360.0;
        s=m_sat/100.0;
        l=m_val/100.0;
        *m_color = this->converter()->fromHslF(h, s, l);
        if (m_hueupdating==false) {
            emit(hueUpdated(static_cast<int>(m_hue)));
        }
        else {
            m_hueupdating=false;
        }
        break;
    case 6:
        m_hue = v;
        h=m_hue/360.0;
        s=m_sat/100.0;
        l=m_val/100.0;
        *m_color = this->converter()->fromHsiF(h, s, l);
        if (m_hueupdating==false) {
            emit(hueUpdated(static_cast<int>(m_hue)));
        }
        else {
            m_hueupdating=false;
        }
        break;
    case 9:
        m_hue = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsyF(h, s, l, R, G, B);
        if (m_hueupdating==false) {
            emit(hueUpdated(static_cast<int>(m_hue)));
        }
        else {
            m_hueupdating=false;
        }
        break;
    case 1:
        m_sat = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsvF(h, s, l);
        if (m_satupdating==false) {
            emit(satUpdated(static_cast<int>(m_sat), m_type));
        }
        else {
            m_satupdating=false;
        }
        break;
    case 2:
        m_val = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsvF(h, s, l);
        if (m_toneupdating==false) {
            emit(toneUpdated(static_cast<int>(m_val), m_type));
        }
        else {
            m_toneupdating=false;
        }
        break;
    case 4:
        m_sat = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHslF(h, s, l);
        if (m_satupdating==false) {
            emit(satUpdated(static_cast<int>(m_sat), m_type));
        }
        else {
            m_satupdating=false;
        }
        break;
    case 5:
        m_val = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHslF(h, s, l);
        if (m_toneupdating==false) {
            emit(toneUpdated(static_cast<int>(m_val), m_type));
        }
        else {
            m_toneupdating=false;
        }
        break;
    case 7:
        m_sat = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsiF(h, s, l);
        if (m_satupdating==false) {
            emit(satUpdated(static_cast<int>(m_sat), m_type));
        }
        else {
            m_satupdating=false;
        }
        break;
    case 8:
        m_val = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsiF(h, s, l);
        if (m_toneupdating==false) {
            emit(toneUpdated(static_cast<int>(m_val), m_type));
        }
        else {
            m_toneupdating=false;
        }
        break;
    case 10:
        m_sat = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsyF(h, s, l, R, G, B);
        if (m_satupdating==false) {
            emit(satUpdated(static_cast<int>(m_sat), m_type));
        }
        else {
            m_satupdating=false;
        }
        break;
    case 11:
        m_val = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsyF(h, s, l, R, G, B);
        if (m_toneupdating==false) {
            emit(toneUpdated(static_cast<int>(m_val), m_type));
        }
        else {
            m_toneupdating=false;
        }
        break;
    default:
        Q_ASSERT(false);
    }
    emit(updated());
}
//update
void KisHSXColorSliderInput::update()
{
    
    KoColor min = *m_color;
    KoColor max = *m_color;
    
    qreal hue, sat, val, hue_backup, sat_backup, val_backup;
    //gets the hsv for the appropriate type//
    hue_backup = m_hue;
    sat_backup = m_sat;
    val_backup = m_val;
    
    switch (m_type) {
    case 0:
        this->converter()->getHsvF(*m_color, &hue, &sat, &val);
        if (m_sliderisupdating==true)
        {
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
            
            
        }
        else{
            if((hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2) {
                hue = (hue_backup/360.0);
            }
        }
        break;
    case 1:
        this->converter()->getHsvF(*m_color, &hue, &sat, &val);
        if (m_sliderisupdating==true)
        {
            if( (hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2 ) {
                hue = (hue_backup/360.0);
            }
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
            
            
        }
        else{
            
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
        }
        break;
    case 2:
        this->converter()->getHsvF(*m_color, &hue, &sat, &val);
        if (m_sliderisupdating==true)
        {
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
            if((hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2) {
                hue = (hue_backup/360.0);
            }
        }
        else{
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
        }
        break;
    case 3:
        this->converter()->getHslF(*m_color, &hue, &sat, &val);
        if (m_sliderisupdating==true)
        {
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
            
            
        }
        else{
            if((hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2) {
                hue = (hue_backup/360.0);
            }
        }
        break;
    case 4:
        this->converter()->getHslF(*m_color, &hue, &sat, &val);
        if (m_sliderisupdating==true)
        {
            if((hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2) {
                hue = (hue_backup/360.0);
            }
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
            
            
        }
        else{
            
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
        }
        break;
    case 5:
        this->converter()->getHslF(*m_color, &hue, &sat, &val);
        if (m_sliderisupdating==true)
        {
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
            if((hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2) {
                hue = (hue_backup/360.0);
            }
        }
        else{
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
        }
        break;
    case 6:
        this->converter()->getHsiF(*m_color, &hue, &sat, &val);
        if (m_sliderisupdating==true)
        {
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
            
            
        }
        else{
            if((hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2) {
                hue = (hue_backup/360.0);
            }
        }
        break;
    case 7:
        this->converter()->getHsiF(*m_color, &hue, &sat, &val);
        if (m_sliderisupdating==true)
        {
            if((hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2) {
                hue = (hue_backup/360.0);
            }
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
            
            
        }
        else{
            
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
        }
        break;
    case 8:
        this->converter()->getHsiF(*m_color, &hue, &sat, &val);
        if (m_sliderisupdating==true)
        {
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
            if((hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2) {
                hue = (hue_backup/360.0);
            }
        }
        else{
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
        }
        break;
    case 9:
        this->converter()->getHsyF(*m_color, &hue, &sat, &val, R, G, B);
        if (m_sliderisupdating==true)
        {
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
            
            
        }
        else{
            if((hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2) {
                hue = (hue_backup/360.0);
            }
        }
        break;
    case 10:
        this->converter()->getHsyF(*m_color, &hue, &sat, &val, R, G, B);
        if (m_sliderisupdating==true)
        {
            if((hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2) {
                hue = (hue_backup/360.0);
            }
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
            
            
        }
        else{
            
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
        }
        break;
    case 11:
        this->converter()->getHsyF(*m_color, &hue, &sat, &val, R, G, B);
        if (m_sliderisupdating == true)
        {
            if((sat*100.0)<m_sat+2 && (sat*100.0)>m_sat-2) {
                sat = (sat_backup*0.01);
            }
            if((hue*360.0)<m_hue+2 && (hue*360.0)>m_hue-2) {
                hue = (hue_backup/360.0);
            }
        }
        else{
            if((val*100.0)<m_val+2 && (val*100.0)>m_val-2) {
                val = (val_backup*0.01);
            }
        }
        break;
    }
    //this prevents the hue going to 0 when used with grey//
    
    if (sat<=0.0) {
        m_hue = hue_backup;
    }
    else{
        m_hue=(hue*360.0);
    }
    
    if (val==0 || val>0.999) {
        m_sat = sat_backup;
    }
    else{
        m_sat=(sat*100.0);
    }
    
    m_val=(val*100.0);
    
    if (m_hueupdating==true){m_val=val_backup; m_sat = sat_backup; m_hueupdating=false;}
    else if (m_satupdating==true){m_val=val_backup; m_hue = hue_backup; m_satupdating=false;}
    else if (m_toneupdating==true){m_sat=sat_backup; m_hue = hue_backup;m_toneupdating=false;}
    
    //sets slider and num-input according to type//
    switch (m_type) {
    case 0:
    case 3:
    case 6:
    case 9:
        m_NumInput->setValue(m_hue);
        m_hsvSlider->setValue(static_cast<int>(m_hue));
        break;
    case 1:
        m_NumInput->setValue(m_sat);
        m_hsvSlider->setValue(static_cast<int>(m_sat));
        break;
    case 2:
        m_NumInput->setValue(m_val);
        m_hsvSlider->setValue(static_cast<int>(m_val));
        break;
    case 4:
        m_NumInput->setValue(m_sat);
        m_hsvSlider->setValue(static_cast<int>(m_sat));
        break;
    case 5:
        m_NumInput->setValue(m_val);
        m_hsvSlider->setValue(static_cast<int>(m_val));
        break;
    case 7:
        m_NumInput->setValue(m_sat);
        m_hsvSlider->setValue(static_cast<int>(m_sat));
        break;
    case 8:
        m_NumInput->setValue(m_val);
        m_hsvSlider->setValue(static_cast<int>(m_val));
        break;
    case 10:
        m_NumInput->setValue(m_sat);
        m_hsvSlider->setValue(static_cast<int>(m_sat));
        break;
    case 11:
        m_NumInput->setValue(m_val);
        m_hsvSlider->setValue(static_cast<int>(m_val));
        break;
    default:
        Q_ASSERT(false);
    }
    m_hsvSlider->setColors(*m_color,m_type, m_hue, R, G, B);
}

QWidget* KisHSXColorSliderInput::createInput()
{
    m_NumInput = new QDoubleSpinBox(this);
    m_NumInput->setMinimum(0);
    m_NumInput->setMaximum(100.0);
    m_NumInput->setKeyboardTracking(false);//this makes sure that only full values are sent after loss of focus. Much more user friendly//
    m_hsvSlider->setMaximum(100);
    switch (m_type) {
    case 0:
    case 3:
    case 6:
    case 9:
        m_NumInput->setMaximum(360.0);
        m_NumInput->setWrapping(true);
        m_hsvSlider->setMaximum(360);
        m_NumInput->setSingleStep (5.0);
        break;
    case 1:
    case 2:
    case 4:
    case 5:
    case 7:
    case 8:
    case 10:
    case 11:
        m_NumInput->setMaximum(100.0);
        m_hsvSlider->setMaximum(100);
        m_NumInput->setSingleStep (10.0);
        break;
    default:
        Q_ASSERT(false);
    }
    connect(m_hsvSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
    connect(m_NumInput, SIGNAL(valueChanged(double)), this, SLOT(numInputChanged(double)));
    return m_NumInput;
}

void KisHSXColorSliderInput::sliderChanged(int i)
{
    m_NumInput->setValue(i*1.0);
    setValue(i*1.0);
}

void KisHSXColorSliderInput::sliderIn(){
    m_sliderisupdating=true;
}

void KisHSXColorSliderInput::sliderOut(){
    m_sliderisupdating=false;
}
//attempt at getting rid of dancing sliders... #2859
//The nminput should not be changing the sliders if the sliders are the one changing the input.
//As numinpit rounds off at 2 decimals(and there's no point at letting it continue the signal circle).
void KisHSXColorSliderInput::numInputChanged(double v)
{
    if (m_sliderisupdating==true){
        return;
    }
    else {
        setValue(v);
    }
}

//this connects to the display converter. Important for OCIO, breaks on missing of m_canvas somehow.
KisDisplayColorConverter* KisHSXColorSliderInput::converter() const
{
    return m_canvas ?
                m_canvas->displayColorConverter() :
                KisDisplayColorConverter::dumbConverterInstance();
}

void KisHSXColorSliderInput::hueUpdate(int h)
{
    if (h<=m_hue-2 || h>=m_hue+2) {
        m_hue=h;
        m_hueupdating=true;
        update();
    }
}

void KisHSXColorSliderInput::satUpdate(int s, int type)
{
    if (m_type==type+1 || m_type==type-1)
    {
        if (s<=m_sat-3 || s>=m_sat+3) {
            m_sat=s;
            m_satupdating=true;
            update();
        }
    }
}

void KisHSXColorSliderInput::toneUpdate(int l, int type)
{
    if (m_type==type-1 || m_type==type-2)
    {
        if (l<25 || l>75){
        
            if (l<=m_val-10 || l>=m_val+10) {
                m_val=l;
                m_toneupdating=true;
                update();
            }
        }
        else {
            if (l<=m_val-3 || l>=m_val+3) {
                m_val=l;
                m_toneupdating=true;
                update();
            }
        }
            
        
    }
}
#include "kis_color_slider_input.moc"
