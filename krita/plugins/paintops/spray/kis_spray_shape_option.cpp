/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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
#include "kis_spray_shape_option.h"
#include <klocale.h>

#include <QImage>

#include "ui_wdgshapeoptions.h"

class KisShapeOptionsWidget: public QWidget, public Ui::WdgShapeOptions
{
public:
    KisShapeOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }

};

KisSprayShapeOption::KisSprayShapeOption()
        : KisPaintOpOption(i18n("Particle type"), false)
{
    m_checkable = false;
    m_options = new KisShapeOptionsWidget();
    connect(m_options->shapeBox, SIGNAL(currentIndexChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->widthSpin, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->heightSpin, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->jitterShape, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->heightPro, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->widthPro, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->proportionalBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->gaussBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));

    connect(m_options->randomSlider,SIGNAL(valueChanged(int)),this,SLOT(randomValueChanged(int)));
    connect(m_options->followSlider,SIGNAL(valueChanged(int)),this,SLOT(followValueChanged(int)));

    connect(m_options->imageUrl,SIGNAL(urlSelected(KUrl)),this,SLOT(prepareImage()));
    
    setConfigurationPage(m_options);
}

KisSprayShapeOption::~KisSprayShapeOption()
{
    // delete m_options;
}

int KisSprayShapeOption::shape() const
{
    return m_options->shapeBox->currentIndex();
}

int KisSprayShapeOption::width() const
{
    return m_options->widthSpin->value();
}

int KisSprayShapeOption::height() const
{
    return m_options->heightSpin->value();
}

bool KisSprayShapeOption::jitterShapeSize() const
{
    return m_options->jitterShape->isChecked();
}

qreal KisSprayShapeOption::heightPerc() const
{
    return m_options->heightPro->value();
}

qreal KisSprayShapeOption::widthPerc() const
{
    return m_options->widthPro->value();
}

bool KisSprayShapeOption::proportional() const
{
    return m_options->proportionalBox->isChecked();
}



// TODO
void KisSprayShapeOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
//     setting->setProperty( "Spray/diameter", diameter() );
}

// TODO
void KisSprayShapeOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    /*    m_options->diameterSpinBox->setValue( setting->getInt("Spray/diameter") );
        m_options->coverageSpin->setValue( setting->getDouble("Spray/coverage") );
        m_options->jitterSizeBox->setChecked( setting->getBool("Spray/jitterSize") );*/
}


bool KisSprayShapeOption::gaussian() const
{
    return m_options->gaussBox->isChecked();
}




QImage KisSprayShapeOption::image() 
{
    m_img = QImage( m_options->imageUrl->url().toLocalFile() );
    m_img = m_img.scaled( width(), height() );
    return m_img;
}



bool KisSprayShapeOption::fixedRotation() const
{
    return m_options->fixedRotation->isChecked();
}


int KisSprayShapeOption::fixedAngle() const
{
    return m_options->fixedRotationSPBox->value();
}


bool KisSprayShapeOption::followCursor() const
{
    return m_options->followCursor->isChecked();
}


qreal KisSprayShapeOption::followCursorWeigth() const
{
    return m_options->followCursorWeightSPBox->value();
}


bool KisSprayShapeOption::randomRotation() const
{
    return m_options->randomRotation->isChecked();
}


qreal KisSprayShapeOption::randomRotationWeight() const
{
    return m_options->randomWeightSPBox->value();
}


void KisSprayShapeOption::randomValueChanged(int value)
{
    qreal relative = value / (qreal)m_options->randomSlider->maximum() ;
    m_options->randomWeightSPBox->setValue( relative * m_options->randomWeightSPBox->maximum() );
}


void KisSprayShapeOption::followValueChanged(int value)
{
    qreal relative = value / (qreal)m_options->followSlider->maximum() ;
    m_options->followCursorWeightSPBox->setValue( relative * m_options->followCursorWeightSPBox->maximum() );
}


void KisSprayShapeOption::prepareImage()
{
    QString path = m_options->imageUrl->url().toLocalFile();
    if (QFile::exists(path)){
        m_img = QImage(path);
        if (!m_img.isNull())
        {
            m_options->widthSpin->setValue( m_img.width() );
            m_options->heightSpin->setValue( m_img.height() );
        }
    }
}