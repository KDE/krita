/*
 *  SPDX-FileCopyrightText: 2010-2011 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_phong_bumpmap_config_widget.h"
#include <filter/kis_filter_configuration.h>
#include <kis_size_group.h>
#include "phong_bumpmap_constants.h"
#include "KoChannelInfo.h"
#include "KoColorSpace.h"
#include <KisGlobalResourcesInterface.h>

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

    //Let widgets warn the preview of when they are updated
    connect(m_page->azimuthAngleSelector1, SIGNAL(angleChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->azimuthAngleSelector2, SIGNAL(angleChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->azimuthAngleSelector3, SIGNAL(angleChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->azimuthAngleSelector4, SIGNAL(angleChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
        
    connect(m_page->lightKColorCombo1, SIGNAL(currentIndexChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->lightKColorCombo2, SIGNAL(currentIndexChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->lightKColorCombo3, SIGNAL(currentIndexChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->lightKColorCombo4, SIGNAL(currentIndexChanged(int)), SIGNAL(sigConfigurationItemChanged()));
    
    connect(m_page->elevationAngleSelector1, SIGNAL(angleChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->elevationAngleSelector2, SIGNAL(angleChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->elevationAngleSelector3, SIGNAL(angleChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));
    connect(m_page->elevationAngleSelector4, SIGNAL(angleChanged(qreal)), SIGNAL(sigConfigurationItemChanged()));

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
    m_page->azimuthAngleSelector1->setAngle( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[0]) );
    m_page->azimuthAngleSelector2->setAngle( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[1]) );
    m_page->azimuthAngleSelector3->setAngle( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[2]) );
    m_page->azimuthAngleSelector4->setAngle( config->getDouble(PHONG_ILLUMINANT_AZIMUTH[3]) );
    m_page->elevationAngleSelector1->setAngle( config->getDouble(PHONG_ILLUMINANT_INCLINATION[0]) );
    m_page->elevationAngleSelector2->setAngle( config->getDouble(PHONG_ILLUMINANT_INCLINATION[1]) );
    m_page->elevationAngleSelector3->setAngle( config->getDouble(PHONG_ILLUMINANT_INCLINATION[2]) );
    m_page->elevationAngleSelector4->setAngle( config->getDouble(PHONG_ILLUMINANT_INCLINATION[3]) );
}

KisPropertiesConfigurationSP KisPhongBumpmapConfigWidget::configuration() const
{
    KisFilterConfigurationSP config = new KisFilterConfiguration("phongbumpmap", 2, KisGlobalResourcesInterface::instance());
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
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[0], m_page->azimuthAngleSelector1->angle());
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[1], m_page->azimuthAngleSelector2->angle());
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[2], m_page->azimuthAngleSelector3->angle());
    config->setProperty(PHONG_ILLUMINANT_AZIMUTH[3], m_page->azimuthAngleSelector4->angle());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[0], m_page->elevationAngleSelector1->angle());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[1], m_page->elevationAngleSelector2->angle());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[2], m_page->elevationAngleSelector3->angle());
    config->setProperty(PHONG_ILLUMINANT_INCLINATION[3], m_page->elevationAngleSelector4->angle());

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

