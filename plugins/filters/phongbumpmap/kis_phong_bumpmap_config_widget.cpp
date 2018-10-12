/*
 *  Copyright (c) 2010-2011 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_phong_bumpmap_config_widget.h"
#include <filter/kis_filter_configuration.h>
#include <kis_size_group.h>
#include "phong_bumpmap_constants.h"
#include "KoChannelInfo.h"
#include "KoColorSpace.h"

KisPhongBumpmapConfigWidget::KisPhongBumpmapConfigWidget(const KisPaintDeviceSP dev, QWidget *parent, Qt::WindowFlags f)
                            : KisConfigWidget(parent, f)
                            , m_device(dev)
{
    Q_ASSERT(m_device);
    m_page = new KisPhongBumpmapWidget(this);

    KisSizeGroup *matPropLabelsGroup = new KisSizeGroup(this);
    matPropLabelsGroup->addWidget(m_page->lblAmbientReflectivity);
    matPropLabelsGroup->addWidget(m_page->lblDiffuseReflectivity);
    matPropLabelsGroup->addWidget(m_page->lblSpecularReflectivity);
    matPropLabelsGroup->addWidget(m_page->lblSpecularShinyExp);

    // Connect widgets to each other
    connect(m_page->azimuthDial1, SIGNAL(valueChanged(int)), m_page->azimuthSpinBox1, SLOT(setValue(int)));
    connect(m_page->azimuthDial2, SIGNAL(valueChanged(int)), m_page->azimuthSpinBox2, SLOT(setValue(int)));
    connect(m_page->azimuthDial3, SIGNAL(valueChanged(int)), m_page->azimuthSpinBox3, SLOT(setValue(int)));
    connect(m_page->azimuthDial4, SIGNAL(valueChanged(int)), m_page->azimuthSpinBox4, SLOT(setValue(int)));
    connect(m_page->azimuthSpinBox1, SIGNAL(valueChanged(int)), m_page->azimuthDial1, SLOT(setValue(int)));
    connect(m_page->azimuthSpinBox2, SIGNAL(valueChanged(int)), m_page->azimuthDial2, SLOT(setValue(int)));
    connect(m_page->azimuthSpinBox3, SIGNAL(valueChanged(int)), m_page->azimuthDial3, SLOT(setValue(int)));
    connect(m_page->azimuthSpinBox4, SIGNAL(valueChanged(int)), m_page->azimuthDial4, SLOT(setValue(int)));

    //Let widgets warn the preview of when they are updated
    connect(m_page->azimuthDial1, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->azimuthDial2, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->azimuthDial3, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->azimuthDial4, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
        
    connect(m_page->lightKColorCombo1, SIGNAL(currentIndexChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->lightKColorCombo2, SIGNAL(currentIndexChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->lightKColorCombo3, SIGNAL(currentIndexChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->lightKColorCombo4, SIGNAL(currentIndexChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    
    connect(m_page->inclinationSpinBox1, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->inclinationSpinBox2, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->inclinationSpinBox3, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->inclinationSpinBox4, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));

    connect(m_page->useNormalMap, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->diffuseReflectivityGroup, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->specularReflectivityGroup, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    
    connect(m_page->ambientReflectivityKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->diffuseReflectivityKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->specularReflectivityKisDoubleSliderSpinBox, SIGNAL(valueChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->shinynessExponentKisSliderSpinBox, SIGNAL(valueChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    
    connect(m_page->heightChannelComboBox, SIGNAL(currentIndexChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    
    connect(m_page->lightSourceGroupBox1, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->lightSourceGroupBox2, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->lightSourceGroupBox3, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->lightSourceGroupBox4, SIGNAL(toggled(bool)), SIGNAL(sigConfigurationItemChanged()));


    QVBoxLayout *l = new QVBoxLayout(this);
    Q_CHECK_PTR(l);

    l->addWidget(m_page);

    /* fill in the channel chooser */
    QList<KoChannelInfo *> channels = m_device->colorSpace()->channels();
    for (quint8 ch = 0; ch < m_device->colorSpace()->colorChannelCount(); ch++)
        m_page->heightChannelComboBox->addItem(channels.at(ch)->name());
    
    connect(m_page->useNormalMap, SIGNAL(toggled(bool)), this, SLOT(slotDisableHeightChannelCombobox(bool)) );


}

void KisPhongBumpmapConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    if (!config) return;
    
    QVariant tempcolor;
    if (config->getBool(USE_NORMALMAP_IS_ENABLED))  {
        m_page->heightChannelComboBox->setEnabled(false);
    } else {
        m_page->heightChannelComboBox->setEnabled(true);
    }
    m_page->ambientReflectivityKisDoubleSliderSpinBox->setValue( config->getDouble(PHONG_AMBIENT_REFLECTIVITY) );
    m_page->diffuseReflectivityKisDoubleSliderSpinBox->setValue( config->getDouble(PHONG_DIFFUSE_REFLECTIVITY) );
    m_page->specularReflectivityKisDoubleSliderSpinBox->setValue( config->getDouble(PHONG_SPECULAR_REFLECTIVITY) );
    m_page->shinynessExponentKisSliderSpinBox->setValue( config->getInt(PHONG_SHINYNESS_EXPONENT) );
    m_page->useNormalMap->setChecked( config->getBool(USE_NORMALMAP_IS_ENABLED) );
    m_page->diffuseReflectivityGroup->setChecked( config->getBool(PHONG_DIFFUSE_REFLECTIVITY_IS_ENABLED) );
    m_page->specularReflectivityGroup->setChecked( config->getBool(PHONG_SPECULAR_REFLECTIVITY_IS_ENABLED) );
    // NOTE: Indexes are off by 1 simply because arrays start at 0 and the GUI naming scheme started at 1
    m_page->lightSourceGroupBox1->setChecked( config->getBool(PHONG_ILLUMINANT_IS_ENABLED[0]) );
    m_page->lightSourceGroupBox2->setChecked( config->getBool(PHONG_ILLUMINANT_IS_ENABLED[1]) );
    m_page->lightSourceGroupBox3->setChecked( config->getBool(PHONG_ILLUMINANT_IS_ENABLED[2]) );
    m_page->lightSourceGroupBox4->setChecked( config->getBool(PHONG_ILLUMINANT_IS_ENABLED[3]) );
    config->getProperty(PHONG_ILLUMINANT_COLOR[0], tempcolor);
    m_page->lightKColorCombo1->setColor(tempcolor.value<QColor>());
    config->getProperty(PHONG_ILLUMINANT_COLOR[1], tempcolor);
    m_page->lightKColorCombo2->setColor(tempcolor.value<QColor>());
    config->getProperty(PHONG_ILLUMINANT_COLOR[2], tempcolor);
    m_page->lightKColorCombo3->setColor(tempcolor.value<QColor>());
    config->getProperty(PHONG_ILLUMINANT_COLOR[3], tempcolor);
    m_page->lightKColorCombo4->setColor(tempcolor.value<QColor>());
    m_page->azimuthSpinBox1->setValue( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[0]) );
    m_page->azimuthSpinBox2->setValue( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[1]) );
    m_page->azimuthSpinBox3->setValue( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[2]) );
    m_page->azimuthSpinBox4->setValue( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[3]) );
    m_page->inclinationSpinBox1->setValue( config->getDouble(PHONG_ILLUMINANT_INCLINATION[0]) );
    m_page->inclinationSpinBox2->setValue( config->getDouble(PHONG_ILLUMINANT_INCLINATION[1]) );
    m_page->inclinationSpinBox3->setValue( config->getDouble(PHONG_ILLUMINANT_INCLINATION[2]) );
    m_page->inclinationSpinBox4->setValue( config->getDouble(PHONG_ILLUMINANT_INCLINATION[3]) );
}

KisPropertiesConfigurationSP KisPhongBumpmapConfigWidget::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("phongbumpmap", 2);
    config->setProperty(PHONG_HEIGHT_CHANNEL, m_page->heightChannelComboBox->currentText());
    config->setProperty(USE_NORMALMAP_IS_ENABLED, m_page->useNormalMap->isChecked());
    
    config->setProperty(PHONG_AMBIENT_REFLECTIVITY, m_page->ambientReflectivityKisDoubleSliderSpinBox->value());
    config->setProperty(PHONG_DIFFUSE_REFLECTIVITY, m_page->diffuseReflectivityKisDoubleSliderSpinBox->value());
    config->setProperty(PHONG_SPECULAR_REFLECTIVITY, m_page->specularReflectivityKisDoubleSliderSpinBox->value());
    config->setProperty(PHONG_SHINYNESS_EXPONENT, m_page->shinynessExponentKisSliderSpinBox->value());
    config->setProperty(PHONG_DIFFUSE_REFLECTIVITY_IS_ENABLED, m_page->diffuseReflectivityGroup->isChecked());
    config->setProperty(PHONG_SPECULAR_REFLECTIVITY_IS_ENABLED, m_page->specularReflectivityGroup->isChecked());
    //config->setProperty(PHONG_SHINYNESS_EXPONENT_IS_ENABLED, m_page->specularReflectivityCheckBox->isChecked());
    // Indexes are off by 1 simply because arrays start at 0 and the GUI naming scheme started at 1
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[0], m_page->lightSourceGroupBox1->isChecked());
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[1], m_page->lightSourceGroupBox2->isChecked());
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[2], m_page->lightSourceGroupBox3->isChecked());
    config->setProperty(PHONG_ILLUMINANT_IS_ENABLED[3], m_page->lightSourceGroupBox4->isChecked());
    config->setProperty(PHONG_ILLUMINANT_COLOR[0], m_page->lightKColorCombo1->color());
    config->setProperty(PHONG_ILLUMINANT_COLOR[1], m_page->lightKColorCombo2->color());
    config->setProperty(PHONG_ILLUMINANT_COLOR[2], m_page->lightKColorCombo3->color());
    config->setProperty(PHONG_ILLUMINANT_COLOR[3], m_page->lightKColorCombo4->color());
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[0], m_page->azimuthSpinBox1->value());
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[1], m_page->azimuthSpinBox2->value());
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[2], m_page->azimuthSpinBox3->value());
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[3], m_page->azimuthSpinBox4->value());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[0], m_page->inclinationSpinBox1->value());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[1], m_page->inclinationSpinBox2->value());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[2], m_page->inclinationSpinBox3->value());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[3], m_page->inclinationSpinBox4->value());

    // Read configuration
    /*
    QMap<QString, QVariant> rofl = QMap<QString, QVariant>(config->getProperties());

    QMap<QString, QVariant>::const_iterator i;
    for (i = rofl.constBegin(); i != rofl.constEnd(); ++i)
        dbgKrita << i.key() << ":" << i.value();
    */
    return config;
}

void KisPhongBumpmapConfigWidget::slotDisableHeightChannelCombobox(bool normalmapchecked) {
    if (normalmapchecked)  {
        m_page->heightChannelComboBox->setEnabled(false);
    } else {
        m_page->heightChannelComboBox->setEnabled(true);
    }
}

