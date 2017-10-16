/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2012
 * Copyright (C) Mohit Goyal <mohit.bits2011@gmail.com>, (C) 2014
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
#include "kis_texture_option.h"

#include <QWidget>
#include <QString>
#include <QByteArray>
#include <QCheckBox>
#include <QBuffer>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QTransform>
#include <QPainter>
#include <QBoxLayout>

#include <klocalizedstring.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_resource_server_provider.h>
#include <kis_pattern_chooser.h>
#include <kis_slider_spin_box.h>
#include <kis_multipliers_double_slider_spinbox.h>
#include <resources/KoPattern.h>
#include <kis_paint_device.h>
#include <kis_fill_painter.h>
#include <kis_painter.h>
#include <kis_iterator_ng.h>
#include <kis_fixed_paint_device.h>
#include <kis_gradient_slider.h>
#include "kis_embedded_pattern_manager.h"
#include "kis_algebra_2d.h"
#include "kis_lod_transform.h"
#include <brushengine/kis_paintop_lod_limitations.h>
#include "kis_texture_chooser.h"
#include <time.h>


KisTextureOption::KisTextureOption()
    : KisPaintOpOption(KisPaintOpOption::TEXTURE, true)
    , m_textureOptions(new KisTextureChooser())
{
    setObjectName("KisTextureOption");
    setConfigurationPage(m_textureOptions);

    connect(m_textureOptions->textureSelectorWidget, SIGNAL(resourceSelected(KoResource*)), SLOT(resetGUI(KoResource*)));
    connect(m_textureOptions->textureSelectorWidget, SIGNAL(resourceSelected(KoResource*)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->scaleSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->brightnessSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->contrastSlider, SIGNAL(valueChanged(qreal)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->offsetSliderX, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->randomOffsetX, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->randomOffsetY, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->offsetSliderY, SIGNAL(valueChanged(int)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->cmbTexturingMode, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->cmbCutoffPolicy, SIGNAL(currentIndexChanged(int)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->cutoffSlider, SIGNAL(sigModifiedBlack(int)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->cutoffSlider, SIGNAL(sigModifiedWhite(int)), SLOT(emitSettingChanged()));
    connect(m_textureOptions->chkInvert, SIGNAL(toggled(bool)), SLOT(emitSettingChanged()));
    resetGUI(m_textureOptions->textureSelectorWidget->currentResource());

}

KisTextureOption::~KisTextureOption()
{
    delete m_textureOptions;
}

void KisTextureOption::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
     m_textureOptions->textureSelectorWidget->blockSignals(true); // Checking
    if (!m_textureOptions->textureSelectorWidget->currentResource()) return;
    KoPattern *pattern = static_cast<KoPattern*>(m_textureOptions->textureSelectorWidget->currentResource());
    m_textureOptions->textureSelectorWidget->blockSignals(false); // Checking
    if (!pattern) return;

    setting->setProperty("Texture/Pattern/Enabled", isChecked());
    if (!isChecked()) {
        return;
    }

    qreal scale = m_textureOptions->scaleSlider->value();

    qreal brightness = m_textureOptions->brightnessSlider->value();

    qreal contrast = m_textureOptions->contrastSlider->value();

    int offsetX = m_textureOptions->offsetSliderX->value();
    if (m_textureOptions ->randomOffsetX->isChecked()) {

        m_textureOptions->offsetSliderX ->setEnabled(false);
        m_textureOptions->offsetSliderX ->blockSignals(true);
        m_textureOptions->offsetSliderX ->setValue(offsetX);
        m_textureOptions->offsetSliderX ->blockSignals(false);
    }
    else {
        m_textureOptions->offsetSliderX ->setEnabled(true);
    }

    int offsetY = m_textureOptions->offsetSliderY->value();
    if (m_textureOptions ->randomOffsetY->isChecked()) {

        m_textureOptions->offsetSliderY ->setEnabled(false);
        m_textureOptions->offsetSliderY ->blockSignals(true);
        m_textureOptions->offsetSliderY ->setValue(offsetY);
        m_textureOptions->offsetSliderY ->blockSignals(false);
    }
    else {
        m_textureOptions->offsetSliderY ->setEnabled(true);
    }

    int texturingMode = m_textureOptions->cmbTexturingMode->currentIndex();
    bool invert = (m_textureOptions->chkInvert->checkState() == Qt::Checked);

    setting->setProperty("Texture/Pattern/Scale", scale);
    setting->setProperty("Texture/Pattern/Brightness", brightness);
    setting->setProperty("Texture/Pattern/Contrast", contrast);
    setting->setProperty("Texture/Pattern/OffsetX", offsetX);
    setting->setProperty("Texture/Pattern/OffsetY", offsetY);
    setting->setProperty("Texture/Pattern/TexturingMode", texturingMode);
    setting->setProperty("Texture/Pattern/CutoffLeft", m_textureOptions->cutoffSlider->black());
    setting->setProperty("Texture/Pattern/CutoffRight", m_textureOptions->cutoffSlider->white());
    setting->setProperty("Texture/Pattern/CutoffPolicy", m_textureOptions->cmbCutoffPolicy->currentIndex());
    setting->setProperty("Texture/Pattern/Invert", invert);

    setting->setProperty("Texture/Pattern/MaximumOffsetX",m_textureOptions->offsetSliderX ->maximum());
    setting->setProperty("Texture/Pattern/MaximumOffsetY",m_textureOptions->offsetSliderY ->maximum());
    setting->setProperty("Texture/Pattern/isRandomOffsetX",m_textureOptions ->randomOffsetX ->isChecked());
    setting->setProperty("Texture/Pattern/isRandomOffsetY",m_textureOptions ->randomOffsetY ->isChecked());

    KisEmbeddedPatternManager::saveEmbeddedPattern(setting, pattern);
}

void KisTextureOption::readOptionSetting(const KisPropertiesConfigurationSP setting)
{

    setChecked(setting->getBool("Texture/Pattern/Enabled"));
    if (!isChecked()) {
        return;
    }
    KoPattern *pattern = KisEmbeddedPatternManager::loadEmbeddedPattern(setting);

    if (!pattern) {
        pattern = static_cast<KoPattern*>(m_textureOptions->textureSelectorWidget->currentResource());
    }

    m_textureOptions->textureSelectorWidget->setCurrentPattern(pattern);

    m_textureOptions->scaleSlider->setValue(setting->getDouble("Texture/Pattern/Scale", 1.0));
    m_textureOptions->brightnessSlider->setValue(setting->getDouble("Texture/Pattern/Brightness"));
    m_textureOptions->contrastSlider->setValue(setting->getDouble("Texture/Pattern/Contrast", 1.0));
    m_textureOptions->offsetSliderX->setValue(setting->getInt("Texture/Pattern/OffsetX"));
    m_textureOptions->offsetSliderY->setValue(setting->getInt("Texture/Pattern/OffsetY"));
    m_textureOptions->randomOffsetX->setChecked(setting->getBool("Texture/Pattern/isRandomOffsetX"));
    m_textureOptions->randomOffsetY->setChecked(setting->getBool("Texture/Pattern/isRandomOffsetY"));
    m_textureOptions->cmbTexturingMode->setCurrentIndex(setting->getInt("Texture/Pattern/TexturingMode", KisTextureProperties::MULTIPLY));
    m_textureOptions->cmbCutoffPolicy->setCurrentIndex(setting->getInt("Texture/Pattern/CutoffPolicy"));
    m_textureOptions->cutoffSlider->slotModifyBlack(setting->getInt("Texture/Pattern/CutoffLeft", 0));
    m_textureOptions->cutoffSlider->slotModifyWhite(setting->getInt("Texture/Pattern/CutoffRight", 255));
    m_textureOptions->chkInvert->setChecked(setting->getBool("Texture/Pattern/Invert"));

}

void KisTextureOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    l->limitations << KoID("texture-pattern", i18nc("PaintOp instant preview limitation", "Texture->Pattern (low quality preview)"));
}


void KisTextureOption::resetGUI(KoResource* res)
{
    KoPattern *pattern = static_cast<KoPattern *>(res);
    if (!pattern) return;

    m_textureOptions->offsetSliderX->setRange(0, pattern->pattern().width() / 2);
    m_textureOptions->offsetSliderY->setRange(0, pattern->pattern().height() / 2);
}

KisTextureProperties::KisTextureProperties(int levelOfDetail)
    : m_pattern(0),
      m_levelOfDetail(levelOfDetail)
{
}

void KisTextureProperties::recalculateMask()
{
    if (!m_pattern) return;

    m_mask = 0;

    QImage mask = m_pattern->pattern();

    if ((mask.format() != QImage::Format_RGB32) |
        (mask.format() != QImage::Format_ARGB32)) {

        mask = mask.convertToFormat(QImage::Format_ARGB32);
    }

    qreal scale = m_scale * KisLodTransform::lodToScale(m_levelOfDetail);

    if (!qFuzzyCompare(scale, 0.0)) {
        QTransform tf;
        tf.scale(scale, scale);
        QRect rc = KisAlgebra2D::ensureRectNotSmaller(tf.mapRect(mask.rect()), QSize(2,2));
        mask = mask.scaled(rc.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    const QRgb* pixel = reinterpret_cast<const QRgb*>(mask.constBits());
    int width = mask.width();
    int height = mask.height();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    m_mask = new KisPaintDevice(cs);

    KisHLineIteratorSP iter = m_mask->createHLineIteratorNG(0, 0, width);

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            const QRgb currentPixel = pixel[row * width + col];

            const int red = qRed(currentPixel);
            const int green = qGreen(currentPixel);
            const int blue = qBlue(currentPixel);
            float alpha = qAlpha(currentPixel) / 255.0;

            const int grayValue = (red * 11 + green * 16 + blue * 5) / 32;
            float maskValue = (grayValue / 255.0) * alpha + (1 - alpha);

            maskValue = maskValue - m_brightness;

            maskValue = ((maskValue - 0.5)*m_contrast)+0.5;

            if (maskValue > 1.0) {maskValue = 1;}
            else if (maskValue < 0) {maskValue = 0;}

            if (m_invert) {
                maskValue = 1 - maskValue;
            }

            if (m_cutoffPolicy == 1 && (maskValue < (m_cutoffLeft / 255.0) || maskValue > (m_cutoffRight / 255.0))) {
                // mask out the dab if it's outside the pattern's cuttoff points
                maskValue = OPACITY_TRANSPARENT_F;
            }
            else if (m_cutoffPolicy == 2 && (maskValue < (m_cutoffLeft / 255.0) || maskValue > (m_cutoffRight / 255.0))) {
                maskValue = OPACITY_OPAQUE_F;
            }

            cs->setOpacity(iter->rawData(), maskValue, 1);
            iter->nextPixel();
        }
        iter->nextRow();
    }

    m_maskBounds = QRect(0, 0, width, height);
}


void KisTextureProperties::fillProperties(const KisPropertiesConfigurationSP setting)
{

    if (!setting->hasProperty("Texture/Pattern/PatternMD5")) {
        m_enabled = false;
        return;
    }

    m_pattern = KisEmbeddedPatternManager::loadEmbeddedPattern(setting);

    if (!m_pattern) {
        warnKrita << "WARNING: Couldn't load the pattern for a stroke";
        m_enabled = false;
        return;
    }

    m_enabled = setting->getBool("Texture/Pattern/Enabled", false);
    m_scale = setting->getDouble("Texture/Pattern/Scale", 1.0);
    m_brightness = setting->getDouble("Texture/Pattern/Brightness");
    m_contrast = setting->getDouble("Texture/Pattern/Contrast");
    m_offsetX = setting->getInt("Texture/Pattern/OffsetX");
    m_offsetY = setting->getInt("Texture/Pattern/OffsetY");
    m_texturingMode = (TexturingMode) setting->getInt("Texture/Pattern/TexturingMode", MULTIPLY);
    m_invert = setting->getBool("Texture/Pattern/Invert");
    m_cutoffLeft = setting->getInt("Texture/Pattern/CutoffLeft", 0);
    m_cutoffRight = setting->getInt("Texture/Pattern/CutoffRight", 255);
    m_cutoffPolicy = setting->getInt("Texture/Pattern/CutoffPolicy", 0);

    m_strengthOption.readOptionSetting(setting);
    m_strengthOption.resetAllSensors();

    recalculateMask();
}

void KisTextureProperties::apply(KisFixedPaintDeviceSP dab, const QPoint &offset, const KisPaintInformation & info)
{
    if (!m_enabled) return;

    KisPaintDeviceSP fillDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    QRect rect = dab->bounds();

    int x = offset.x() % m_maskBounds.width() - m_offsetX;
    int y = offset.y() % m_maskBounds.height() - m_offsetY;

    KisFillPainter fillPainter(fillDevice);
    fillPainter.fillRect(x - 1, y - 1, rect.width() + 2, rect.height() + 2, m_mask, m_maskBounds);
    fillPainter.end();

    qreal pressure = m_strengthOption.apply(info);
    quint8 *dabData = dab->data();

    KisHLineIteratorSP iter = fillDevice->createHLineIteratorNG(x, y, rect.width());
    for (int row = 0; row < rect.height(); ++row) {
        for (int col = 0; col < rect.width(); ++col) {
            if (m_texturingMode == MULTIPLY) {
                dab->colorSpace()->multiplyAlpha(dabData, quint8(*iter->oldRawData() * pressure), 1);
            }
            else {
                int pressureOffset = (1.0 - pressure) * 255;

                qint16 maskA = *iter->oldRawData() + pressureOffset;
                quint8 dabA = dab->colorSpace()->opacityU8(dabData);

                dabA = qMax(0, (qint16)dabA - maskA);
                dab->colorSpace()->setOpacity(dabData, dabA, 1);
            }

            iter->nextPixel();
            dabData += dab->pixelSize();
        }
        iter->nextRow();
    }
}

