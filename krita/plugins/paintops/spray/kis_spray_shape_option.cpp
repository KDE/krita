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

#include <KoImageResource.h>

#include <QImage>

#include "ui_wdgshapeoptions.h"

class KisShapeOptionsWidget: public QWidget, public Ui::WdgShapeOptions
{
public:
    KisShapeOptionsWidget(QWidget *parent = 0)
            : QWidget(parent) {
        setupUi(this);
        KoImageResource kir;
        btnAspect->setIcon(QIcon(kir.chain()));
    }

};

KisSprayShapeOption::KisSprayShapeOption()
        : KisPaintOpOption(i18n("Particle type"), false)
{
    m_checkable = false;
    m_options = new KisShapeOptionsWidget();
    m_useAspect = m_options->btnAspect->isChecked();
    computeAspect();

    // UI signals
    connect(m_options->btnAspect, SIGNAL(toggled(bool)), this, SLOT(aspectToggled(bool)));
    connect(m_options->randomSlider,SIGNAL(valueChanged(int)),this,SLOT(randomValueChanged(int)));
    connect(m_options->followSlider,SIGNAL(valueChanged(int)),this,SLOT(followValueChanged(int)));
    connect(m_options->imageUrl,SIGNAL(textChanged(QString)),this,SLOT(prepareImage()));

    connect(m_options->widthSpin, SIGNAL(valueChanged(int)), SLOT(updateHeight(int)));
    connect(m_options->heightSpin, SIGNAL(valueChanged(int)), SLOT(updateWidth(int)));

    setConfigurationPage(m_options);
}


void KisSprayShapeOption::setupBrushPreviewSignals()
{
    connect(m_options->shapeBox, SIGNAL(currentIndexChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->widthSpin, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->heightSpin, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_options->jitterShape, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->heightPro, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->widthPro, SIGNAL(valueChanged(double)), SIGNAL(sigSettingChanged()));
    connect(m_options->proportionalBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    connect(m_options->gaussBox, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
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
    Q_UNUSED(setting);
}

// TODO
void KisSprayShapeOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    Q_UNUSED(setting);
}


bool KisSprayShapeOption::gaussian() const
{
    return m_options->gaussBox->isChecked();
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
        m_image = QImage(path);
        if (!m_image.isNull())
        {
            m_options->widthSpin->setValue( m_image.width() );
            m_options->heightSpin->setValue( m_image.height() );
        }
    }
}


void KisSprayShapeOption::aspectToggled(bool toggled)
{
    m_useAspect = toggled;
    KoImageResource kir;
    if (toggled){
        m_options->btnAspect->setIcon(QIcon(kir.chain()));
    } else {
        m_options->btnAspect->setIcon(QIcon(kir.chainBroken()));
    }
}



void KisSprayShapeOption::updateHeight(int value)
{
    if (m_useAspect){
        int newHeight = qRound(value * 1.0/m_aspect);
        m_options->heightSpin->blockSignals(true);
        m_options->heightSpin->setValue(newHeight);
        m_options->heightSlider->setValue(newHeight);
        m_options->heightSpin->blockSignals(false);
    }else{
        computeAspect();
    }
}


void KisSprayShapeOption::updateWidth(int value)
{
    if (m_useAspect){
        int newWidth = qRound(value * m_aspect);
        m_options->widthSpin->blockSignals(true);
        m_options->widthSpin->setValue( newWidth );
        m_options->widthSlider->setValue( newWidth );
        m_options->widthSpin->blockSignals(false);
    }else{
        computeAspect();
    }
}


void KisSprayShapeOption::computeAspect()
{
    qreal w = m_options->widthSpin->value();
    qreal h = m_options->heightSpin->value();
    m_aspect = w / h;
}
