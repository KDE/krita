/*
 *  Copyright (c) 2008,2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#include <klocalizedstring.h>

#include <QImage>
#include <QFile>
#include <KisImportExportManager.h>
#include <kis_file_name_requester.h>
#include "kis_aspect_ratio_locker.h"
#include "kis_signals_blocker.h"

#include "ui_wdgsprayshapeoptions.h"

class KisShapeOptionsWidget: public QWidget, public Ui::WdgSprayShapeOptions
{
public:
    KisShapeOptionsWidget(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
        imageUrl->setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
    }

};

KisSprayShapeOption::KisSprayShapeOption()
    : KisPaintOpOption(KisPaintOpOption::GENERAL, true),
      m_sizeRatioLocker(new KisAspectRatioLocker(this))
{

    setObjectName("KisSprayShapeOption");

    m_checkable = true;
    // save this to be able to restore it back
    m_maxSize = 1000;

    m_options = new KisShapeOptionsWidget();

    //initialize slider values
    m_options->widthSpin->setRange(1, 1000, 0);
    m_options->widthSpin->setValue(6);
    m_options->widthSpin->setSuffix(i18n(" px"));

    m_options->heightSpin->setRange(1, 1000, 0);
    m_options->heightSpin->setValue(6);
    m_options->heightSpin->setSuffix(i18n(" px"));

    // UI signals
    connect(m_options->proportionalBox, SIGNAL(clicked(bool)), SLOT(changeSizeUI(bool)));
    connect(m_options->imageUrl, SIGNAL(textChanged(QString)), this, SLOT(prepareImage()));

    m_sizeRatioLocker->connectSpinBoxes(m_options->widthSpin, m_options->heightSpin, m_options->aspectButton);
    m_sizeRatioLocker->setBlockUpdateSignalOnDrag(true);
    connect(m_sizeRatioLocker, SIGNAL(sliderValueChanged()), SLOT(emitSettingChanged()));
    connect(m_sizeRatioLocker, SIGNAL(aspectButtonChanged()), SLOT(emitSettingChanged()));

    connect(m_options->proportionalBox, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_options->proportionalBox, SIGNAL(clicked(bool)), SLOT(emitSettingChanged()));
    connect(m_options->shapeBox, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
    connect(m_options->imageUrl, SIGNAL(textChanged(QString)), SLOT(emitSettingChanged()));

    setConfigurationPage(m_options);
}

KisSprayShapeOption::~KisSprayShapeOption()
{
    delete m_options;
}

int KisSprayShapeOption::shape() const
{
    return m_options->shapeBox->currentIndex();
}


void KisSprayShapeOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    setting->setProperty(SPRAYSHAPE_ENABLED, isChecked());
    setting->setProperty(SPRAYSHAPE_SHAPE, shape());
    setting->setProperty(SPRAYSHAPE_USE_ASPECT, m_options->aspectButton->keepAspectRatio());
    setting->setProperty(SPRAYSHAPE_PROPORTIONAL, m_options->proportionalBox->isChecked());
    setting->setProperty(SPRAYSHAPE_WIDTH, m_options->widthSpin->value());
    setting->setProperty(SPRAYSHAPE_HEIGHT, m_options->heightSpin->value());
    setting->setProperty(SPRAYSHAPE_IMAGE_URL, m_options->imageUrl->fileName());
}


void KisSprayShapeOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    // in 2.2 there is not shape enabled so true by default
    setChecked(setting->getBool(SPRAYSHAPE_ENABLED, true));
    m_options->shapeBox->setCurrentIndex(setting->getInt(SPRAYSHAPE_SHAPE));
    m_options->proportionalBox->setChecked(setting->getBool(SPRAYSHAPE_PROPORTIONAL));
    m_options->aspectButton->setKeepAspectRatio(setting->getBool(SPRAYSHAPE_USE_ASPECT, false));
    m_options->widthSpin->setValue(setting->getInt(SPRAYSHAPE_WIDTH));
    m_options->heightSpin->setValue(setting->getInt(SPRAYSHAPE_HEIGHT));
    m_options->imageUrl->setFileName(setting->getString(SPRAYSHAPE_IMAGE_URL));
}


void KisSprayShapeOption::prepareImage()
{
    QString path = m_options->imageUrl->fileName();
    if (QFile::exists(path)) {
        QImage image(path);
        if (!image.isNull()) {
            KisSignalsBlocker b(m_options->widthSpin, m_options->heightSpin);
            m_options->widthSpin->setValue(image.width());
            m_options->heightSpin->setValue(image.height());
        }
    }
}

void KisSprayShapeOption::changeSizeUI(bool proportionalSize)
{
    // if proportionalSize is false, pixel size is used
    if (!proportionalSize) {
        m_options->widthSpin->setMaximum(m_maxSize);
        m_options->widthSpin->setSuffix(i18n(" px"));
        m_options->heightSpin->setMaximum(m_maxSize);
        m_options->heightSpin->setSuffix(i18n(" px"));
    }
    else {
        m_options->widthSpin->setMaximum(100);
        m_options->widthSpin->setSuffix("%");
        m_options->heightSpin->setMaximum(100);
        m_options->heightSpin->setSuffix("%");
    }
}
