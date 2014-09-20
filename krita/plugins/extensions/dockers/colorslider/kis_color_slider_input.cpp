/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Wolthera van Hövell <griffinvalley@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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

#include <QGridLayout>
#include <QLabel>
#include <QFontMetrics>
 
#include <klocale.h>
#include <kconfiggroup.h>
#include <kcomponentdata.h>
#include <kglobal.h>
#include <QDoubleSpinBox>


#include <KoColor.h>
#include <KoColorSpace.h>

#include "kis_hsv_slider.h"
#include "kis_display_color_converter.h"

KisColorSliderInput::KisColorSliderInput(QWidget* parent, KoColor* color, const int type, KoColorDisplayRendererInterface *displayRenderer, KisCanvas2* canvas) : QWidget(parent), 
m_type(type),
m_color(color), 
m_displayRenderer(displayRenderer),
m_canvas(canvas)
{
//init
}

void KisColorSliderInput::init()
{
    QGridLayout* m_layout = new QGridLayout(this);

    QString m_name;
    switch (m_type){
    case 0: m_name="Hue"; break;
    case 1: m_name="Saturation"; break;
    case 2: m_name="Value"; break;
    case 3: m_name="Hue"; break;
    case 4: m_name="Saturation"; break;
    case 5: m_name="Lightness"; break;
    case 6: m_name="Hue"; break;
    case 7: m_name="Saturation"; break;
    case 8: m_name="Intensity"; break;
    case 9: m_name="Hue"; break;
    case 10: m_name="Saturation"; break;
    case 11: m_name="Luma"; break;
    }
    
    QLabel* m_label = new QLabel(i18n("%1:", m_name), this);
    //QFontMetrics font =  new QFontMetrics();
    //font = m_label->fontMetrics();
    int max_width = 60;
    //m_label->setMaximumWidth(60);
    m_label->setMinimumWidth(max_width);
    m_label->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    m_layout->addWidget(m_label, 0, 0, Qt::AlignLeft);

    m_hsvSlider = new KisHSVSlider(Qt::Horizontal, this, m_displayRenderer, m_canvas);
    m_hsvSlider->setMaximumHeight(60);
    m_hsvSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_layout->addWidget(m_hsvSlider, 0, 1);

    QWidget* m_input = createInput();
    m_input->setMaximumHeight(60);
    m_input->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_layout->addWidget(m_input, 0, 2, Qt::AlignRight);
}

KisHSXColorSliderInput::KisHSXColorSliderInput(QWidget* parent, const int type, KoColor* color, KoColorDisplayRendererInterface *displayRenderer, KisCanvas2* canvas) : KisColorSliderInput(parent, color, type, displayRenderer, canvas),
m_canvas(canvas)
{
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
        break;
    case 2:
        m_val = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsvF(h, s, l);
        break;
    case 4:
        m_sat = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHslF(h, s, l);
        break;
    case 5:
        m_val = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHslF(h, s, l);
        break;
    case 7:
        m_sat = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsiF(h, s, l);
        break;
    case 8:
        m_val = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsiF(h, s, l);
        break;
    case 10:
        m_sat = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsyF(h, s, l, R, G, B);
        break;
    case 11:
        m_val = v;
        h=m_hue/360.0f;
        s=m_sat/100.0f;
        l=m_val/100.0f;
        *m_color = this->converter()->fromHsyF(h, s, l, R, G, B);
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
    
    qreal hue, sat, val, hue_backup, sat_backup;
    //gets the hsv for the appropriate type//
    hue_backup = m_hue;
    sat_backup = m_sat;
    
    switch (m_type) {
    case 0:
    case 1:
    case 2:
        this->converter()->getHsvF(*m_color, &hue, &sat, &val);
        break;
    case 3:
    case 4:
    case 5:
        this->converter()->getHslF(*m_color, &hue, &sat, &val);
        break;
    case 6:
    case 7:
    case 8:
        this->converter()->getHsiF(*m_color, &hue, &sat, &val);
        break;
    case 9:
    case 10:
    case 11:
        this->converter()->getHsyF(*m_color, &hue, &sat, &val, R, G, B);
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
        break;
    default:
        Q_ASSERT(false);
    }
    connect(m_hsvSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderChanged(int)));
    connect(m_NumInput, SIGNAL(valueChanged(double)), this, SLOT(setValue(double)));
    return m_NumInput;
}

void KisHSXColorSliderInput::sliderChanged(int i)
{
    m_NumInput->setValue(i*1.0);
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
    if (h<=m_hue-1 || h>=m_hue+1) {
        m_hue=h;
        m_hueupdating=true;
        update();
    }
}

#include "kis_color_slider_input.moc"
