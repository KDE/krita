/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_dlg_options_tiff.h"

#include <QCheckBox>
#include <QGroupBox>
#include <QSlider>
#include <QStackedWidget>
#include <QApplication>

#include <kcombobox.h>
#include <klocalizedstring.h>

#include <KoColorSpace.h>
#include <KoChannelInfo.h>
#include <KoColorModelStandardIds.h>

#include <kis_properties_configuration.h>
#include <kis_config.h>

KisTIFFOptionsWidget::KisTIFFOptionsWidget(QWidget *parent)
    : KisConfigWidget(parent)
{
    setupUi(this);
    activated(0);
    connect(kComboBoxCompressionType, SIGNAL(activated(int)), this, SLOT(activated(int)));
    connect(flatten, SIGNAL(toggled(bool)), this, SLOT(flattenToggled(bool)));
    QApplication::restoreOverrideCursor();
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));
}

KisTIFFOptionsWidget::~KisTIFFOptionsWidget()
{
    delete optionswdg;
}

void KisTIFFOptionsWidget::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    kComboBoxCompressionType->setCurrentIndex(cfg->getInt("compressiontype", 0));
    activated(kComboBoxCompressionType->currentIndex());
    kComboBoxPredictor->setCurrentIndex(cfg->getInt("predictor", 0));
    alpha->setChecked(cfg->getBool("alpha", true));
    flatten->setChecked(cfg->getBool("flatten", true));
    flattenToggled(flatten->isChecked());
    qualityLevel->setValue(cfg->getInt("quality", 80));
    compressionLevelDeflate->setValue(cfg->getInt("deflate", 6));
    kComboBoxFaxMode->setCurrentIndex(cfg->getInt("faxmode", 0));
    compressionLevelPixarLog->setValue(cfg->getInt("pixarlog", 6));
    chkSaveProfile->setChecked(cfg->getBool("saveProfile", true));

    if (cfg->getInt("type", -1) == KoChannelInfo::FLOAT16 || cfg->getInt("type", -1) == KoChannelInfo::FLOAT32) {
        kComboBoxPredictor->removeItem(1);
    } else {
        kComboBoxPredictor->removeItem(2);
    }

    if (cfg->getBool("isCMYK")) {
        alpha->setChecked(false);
        alpha->setEnabled(false);
    }


}

KisPropertiesConfigurationSP KisTIFFOptionsWidget::configuration() const
{
    KisTIFFOptions opts = options();
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());
    cfg->setProperty("compressiontype", kComboBoxCompressionType->currentIndex());
    cfg->setProperty("predictor", opts.predictor - 1);
    cfg->setProperty("alpha", opts.alpha);
    cfg->setProperty("flatten", opts.flatten);
    cfg->setProperty("quality", opts.jpegQuality);
    cfg->setProperty("deflate", opts.deflateCompress);
    cfg->setProperty("faxmode", opts.faxMode - 1);
    cfg->setProperty("pixarlog", opts.pixarLogCompress);
    cfg->setProperty("saveProfile", opts.saveProfile);

    return cfg;
}

void KisTIFFOptionsWidget::activated(int index)
{
    switch (index) {
    case 1:
        codecsOptionsStack->setCurrentIndex(1);
        break;
    case 2:
        codecsOptionsStack->setCurrentIndex(2);
        break;
    case 6:
        codecsOptionsStack->setCurrentIndex(3);
        break;
    case 8:
        codecsOptionsStack->setCurrentIndex(4);
        break;
    default:
        codecsOptionsStack->setCurrentIndex(0);
    }
}

void KisTIFFOptionsWidget::flattenToggled(bool t)
{
    alpha->setEnabled(t);
    if (!t) {
        alpha->setChecked(true);
    }
}

KisTIFFOptions KisTIFFOptionsWidget::options() const
{
    KisTIFFOptions options;
    switch (kComboBoxCompressionType->currentIndex()) {
    case 0:
        options.compressionType = COMPRESSION_NONE;
        break;
    case 1:
        options.compressionType = COMPRESSION_JPEG;
        break;
    case 2:
        options.compressionType = COMPRESSION_DEFLATE;
        break;
    case 3:
        options.compressionType = COMPRESSION_LZW;
        break;
    case 4:
        options.compressionType = COMPRESSION_JP2000;
        break;
    case 5:
        options.compressionType = COMPRESSION_CCITTRLE;
        break;
    case 6:
        options.compressionType = COMPRESSION_CCITTFAX3;
        break;
    case 7:
        options.compressionType = COMPRESSION_CCITTFAX4;
        break;
    case 8:
        options.compressionType = COMPRESSION_PIXARLOG;
        break;
    default:
        options.compressionType = COMPRESSION_NONE;
    }
    options.predictor = kComboBoxPredictor->currentIndex() + 1;
    options.alpha = alpha->isChecked();
    options.flatten = flatten->isChecked();
    options.jpegQuality = qualityLevel->value();
    options.deflateCompress = compressionLevelDeflate->value();
    options.faxMode = kComboBoxFaxMode->currentIndex() + 1;
    options.pixarLogCompress = compressionLevelPixarLog->value();
    options.saveProfile = chkSaveProfile->isChecked();

    return options;
}

