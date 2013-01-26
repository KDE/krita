/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2012
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

#include <klocale.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_resource_server_provider.h>
#include <kis_pattern_chooser.h>
#include <kis_slider_spin_box.h>
#include <kis_multipliers_double_slider_spinbox.h>
#include <kis_pattern.h>
#include <kis_paint_device.h>
#include <kis_fill_painter.h>
#include <kis_painter.h>
#include <kis_iterator_ng.h>
#include <kis_fixed_paint_device.h>
#include <kis_gradient_slider.h>

class KisTextureOptionWidget : public QWidget
{
public:

    KisTextureOptionWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        QFormLayout *formLayout = new QFormLayout(this);
        chooser = new KisPatternChooser(this);
        chooser->setGrayscalePreview(true);
        chooser->setMaximumHeight(250);
        chooser->setCurrentItem(0, 0);
        formLayout->addRow(chooser);

        scaleSlider = new KisMultipliersDoubleSliderSpinBox(this);
        scaleSlider->setRange(0.0, 2.0, 2);
        scaleSlider->setValue(1.0);
        scaleSlider->addMultiplier(0.1);
        scaleSlider->addMultiplier(2);
        scaleSlider->addMultiplier(10);

        formLayout->addRow(i18n("Scale:"), scaleSlider);

        offsetSliderX = new KisSliderSpinBox(this);
        offsetSliderX->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        formLayout->addRow(i18n("Horizontal Offset:"), offsetSliderX);

        offsetSliderY = new KisSliderSpinBox(this);
        offsetSliderY->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        formLayout->addRow(i18n("Vertical Offset:"), offsetSliderY);

        cmbTexturingMode = new QComboBox(this);
        cmbTexturingMode->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QStringList texturingModes;
        texturingModes << i18n("Multiply") << i18n("Subtract");
        cmbTexturingMode->addItems(texturingModes);
        formLayout->addRow(i18n("Texturing Mode:"), cmbTexturingMode);
        cmbTexturingMode->setCurrentIndex(KisTextureProperties::SUBTRACT);

        cmbCutoffPolicy = new QComboBox(this);
        cmbCutoffPolicy->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QStringList cutOffPolicies;
        cutOffPolicies << i18n("Cut Off Disabled") << i18n("Cut Off Brush") << i18n("Cut Off Pattern");
        cmbCutoffPolicy->addItems(cutOffPolicies);
        formLayout->addRow(i18n("Cutoff Policy"), cmbCutoffPolicy);

        cutoffSlider = new KisGradientSlider(this);
        cutoffSlider->setMinimumSize(256, 30);
        cutoffSlider->enableGamma(false);
        cutoffSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        cutoffSlider->setToolTip(i18n("When pattern texture values are outside the range specified"
                                      " by the slider, the cut-off policy will be applied."));
        formLayout->addRow(i18n("Cutoff"), cutoffSlider);

        chkInvert = new QCheckBox("", this);
        chkInvert->setChecked(false);
        formLayout->addRow(i18n("Invert Texture:"), chkInvert);

        setLayout(formLayout);
    }
    KisPatternChooser *chooser;
    KisMultipliersDoubleSliderSpinBox *scaleSlider;
    KisSliderSpinBox *offsetSliderX;
    KisSliderSpinBox *offsetSliderY;
    QComboBox *cmbTexturingMode;
    KisGradientSlider *cutoffSlider;
    QComboBox *cmbCutoffPolicy;
    QCheckBox *chkInvert;
};

KisTextureOption::KisTextureOption(QObject *)
    : KisPaintOpOption(i18n("Pattern"), KisPaintOpOption::textureCategory(), true)
{
    setChecked(false);
    m_optionWidget = new KisTextureOptionWidget;
    m_optionWidget->hide();
    setConfigurationPage(m_optionWidget);

    connect(m_optionWidget->chooser, SIGNAL(resourceSelected(KoResource*)), SLOT(resetGUI(KoResource*)));
    connect(m_optionWidget->chooser, SIGNAL(resourceSelected(KoResource*)), SIGNAL(sigSettingChanged()));
    connect(m_optionWidget->scaleSlider, SIGNAL(valueChanged(qreal)), SIGNAL(sigSettingChanged()));
    connect(m_optionWidget->offsetSliderX, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_optionWidget->offsetSliderY, SIGNAL(valueChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_optionWidget->cmbTexturingMode, SIGNAL(currentIndexChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_optionWidget->cmbCutoffPolicy, SIGNAL(currentIndexChanged(int)), SIGNAL(sigSettingChanged()));
    connect(m_optionWidget->cutoffSlider, SIGNAL(sigModifiedBlack(int)), SIGNAL(sigSettingChanged()));
    connect(m_optionWidget->cutoffSlider, SIGNAL(sigModifiedWhite(int)), SIGNAL(sigSettingChanged()));
    connect(m_optionWidget->chkInvert, SIGNAL(toggled(bool)), SIGNAL(sigSettingChanged()));
    resetGUI(m_optionWidget->chooser->currentResource());
}

KisTextureOption::~KisTextureOption()
{
    delete m_optionWidget;
}

void KisTextureOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    if (!m_optionWidget->chooser->currentResource()) return;
    KisPattern *pattern = static_cast<KisPattern*>(m_optionWidget->chooser->currentResource());
    if (!pattern) return;

    qreal scale = m_optionWidget->scaleSlider->value();
    int offsetX = m_optionWidget->offsetSliderX->value();
    int offsetY = m_optionWidget->offsetSliderY->value();
    int texturingMode = m_optionWidget->cmbTexturingMode->currentIndex();
    bool invert = (m_optionWidget->chkInvert->checkState() == Qt::Checked);

    setting->setProperty("Texture/Pattern/Scale", scale);
    setting->setProperty("Texture/Pattern/OffsetX", offsetX);
    setting->setProperty("Texture/Pattern/OffsetY", offsetY);
    setting->setProperty("Texture/Pattern/TexturingMode", texturingMode);
    setting->setProperty("Texture/Pattern/CutoffLeft", m_optionWidget->cutoffSlider->black());
    setting->setProperty("Texture/Pattern/CutoffRight", m_optionWidget->cutoffSlider->white());
    setting->setProperty("Texture/Pattern/CutoffPolicy", m_optionWidget->cmbCutoffPolicy->currentIndex());
    setting->setProperty("Texture/Pattern/Invert", invert);

    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    pattern->image().save(&buffer, "PNG");

    setting->setProperty("Texture/Pattern/Pattern", ba.toBase64());
    setting->setProperty("Texture/Pattern/PatternFileName", pattern->filename());
    setting->setProperty("Texture/Pattern/Name", pattern->name());

    setting->setProperty("Texture/Pattern/Enabled", isChecked());
}

void KisTextureOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    QByteArray ba = QByteArray::fromBase64(setting->getString("Texture/Pattern/Pattern").toLatin1());
    QImage img;
    img.loadFromData(ba, "PNG");
    QString name = setting->getString("Texture/Pattern/Name");
    if (name.isEmpty()) {
        name = setting->getString("Texture/Pattern/FileName");
    }

    KisPattern *pattern = 0;
    if (!img.isNull()) {
        pattern = new KisPattern(img, name);
    }
    // now check whether the pattern already occurs, if not, add it to the
    // resources.
    if (pattern) {
        bool found = false;
        foreach(KoResource *res, KisResourceServerProvider::instance()->patternServer()->resources()) {
            KisPattern *pat = static_cast<KisPattern *>(res);
            if (pat->md5() == pattern->md5()) {
                delete pattern;
                pattern = pat;
                found = true;
                break;
            }
        }
        if (!found) {
            KisResourceServerProvider::instance()->patternServer()->addResource(pattern, false);
        }
        m_optionWidget->offsetSliderX->setRange(0, pattern->image().width() / 2);
        m_optionWidget->offsetSliderY->setRange(0, pattern->image().height() / 2);
    }
    else {
        pattern = static_cast<KisPattern*>(m_optionWidget->chooser->currentResource());
    }
    m_optionWidget->chooser->setCurrentPattern(pattern);

    m_optionWidget->scaleSlider->setValue(setting->getDouble("Texture/Pattern/Scale", 1.0));
    m_optionWidget->offsetSliderX->setValue(setting->getInt("Texture/Pattern/OffsetX"));
    m_optionWidget->offsetSliderY->setValue(setting->getInt("Texture/Pattern/OffsetY"));
    m_optionWidget->cmbTexturingMode->setCurrentIndex(setting->getInt("Texture/Pattern/TexturingMode", KisTextureProperties::MULTIPLY));
    m_optionWidget->cmbCutoffPolicy->setCurrentIndex(setting->getInt("Texture/Pattern/CutoffPolicy"));
    m_optionWidget->cutoffSlider->slotModifyBlack(setting->getInt("Texture/Pattern/CutoffLeft", 0));
    m_optionWidget->cutoffSlider->slotModifyWhite(setting->getInt("Texture/Pattern/CutoffRight", 255));
    m_optionWidget->chkInvert->setChecked(setting->getBool("Texture/Pattern/Invert"));

    setChecked(setting->getBool("Texture/Pattern/Enabled"));
}

void KisTextureOption::resetGUI(KoResource* res)
{
    KisPattern *pattern = static_cast<KisPattern *>(res);
    m_optionWidget->offsetSliderX->setRange(0, pattern->image().width() / 2);
    m_optionWidget->offsetSliderY->setRange(0, pattern->image().height() / 2);
}

void KisTextureProperties::recalculateMask()
{
    if (!pattern) return;

    m_mask = 0;

    QImage mask = pattern->image();

    if (!qFuzzyCompare(scale, 0.0)) {
        QTransform tf;
        tf.scale(scale, scale);
        QRect rc = tf.mapRect(mask.rect());
        mask = mask.scaled(rc.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QRgb* pixel = reinterpret_cast<QRgb*>( mask.bits() );
    int width = mask.width();
    int height = mask.height();

    const KoColorSpace *cs = KoColorSpaceRegistry::instance()->alpha8();
    m_mask = new KisPaintDevice(cs);

    KisHLineIteratorSP iter = m_mask->createHLineIteratorNG(0, 0, width);

    for (int row = 0; row < height; ++row ) {
        for (int col = 0; col < width; ++col ) {
            const QRgb currentPixel = pixel[row * width + col];

            const int red = qRed(currentPixel);
            const int green = qGreen(currentPixel);
            const int blue = qBlue(currentPixel);
            float alpha = qAlpha(currentPixel) / 255.0;

            const int grayValue = (red * 11 + green * 16 + blue * 5) / 32;
            float maskValue = (grayValue / 255.0) * alpha + (1 - alpha);

            if (invert) {
                maskValue = 1 - maskValue;
            }

            if (cutoffPolicy == 1 && (maskValue < (cutoffLeft / 255.0) || maskValue > (cutoffRight / 255.0))) {
                // mask out the dab if it's outside the pattern's cuttoff points
                maskValue = OPACITY_TRANSPARENT_F;
            } else if (cutoffPolicy == 2 && (maskValue < (cutoffLeft / 255.0) || maskValue > (cutoffRight / 255.0))) {
                maskValue = OPACITY_OPAQUE_F;
            }

            cs->setOpacity(iter->rawData(), maskValue, 1);
            iter->nextPixel();
        }
        iter->nextRow();
    }

    m_maskBounds = QRect(0, 0, width, height);
}


void KisTextureProperties::fillProperties(const KisPropertiesConfiguration *setting)
{

    QByteArray ba = QByteArray::fromBase64(setting->getString("Texture/Pattern/Pattern").toLatin1());
    QImage img;
    img.loadFromData(ba, "PNG");
    QString name = setting->getString("Texture/Pattern/Name");
    if (name.isEmpty()) {
        name = setting->getString("Texture/Pattern/FileName");
    }

    pattern = 0;
    if (!img.isNull()) {
        pattern = new KisPattern(img, name);
    }
    if (pattern) {
        foreach(KoResource *res, KisResourceServerProvider::instance()->patternServer()->resources()) {
            KisPattern *pat = static_cast<KisPattern *>(res);
            if (pat == pattern) {
                delete pattern;
                pattern = pat;
                break;
            }
        }
    }

    enabled = setting->getBool("Texture/Pattern/Enabled", false);
    scale = setting->getDouble("Texture/Pattern/Scale", 1.0);
    offsetX = setting->getInt("Texture/Pattern/OffsetX");
    offsetY = setting->getInt("Texture/Pattern/OffsetY");
    texturingMode = (TexturingMode) setting->getInt("Texture/Pattern/TexturingMode", MULTIPLY);
    invert = setting->getBool("Texture/Pattern/Invert");
    cutoffLeft = setting->getInt("Texture/Pattern/CutoffLeft", 0);
    cutoffRight = setting->getInt("Texture/Pattern/CutoffRight", 255);
    cutoffPolicy = setting->getInt("Texture/Pattern/CutoffPolicy", 0);

    m_strengthOption.readOptionSetting(setting);

    recalculateMask();
}

void KisTextureProperties::apply(KisFixedPaintDeviceSP dab, const QPoint &offset, const KisPaintInformation & info)
{
    if (!enabled) return;

    KisPaintDeviceSP fillDevice = new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());
    QRect rect = dab->bounds();

    int x = offset.x() % m_maskBounds.width() - offsetX;
    int y = offset.y() % m_maskBounds.height() - offsetY;

    fillDevice->setX(x);
    fillDevice->setY(y);

    KisFillPainter fillPainter(fillDevice);
    fillPainter.fillRect(x - 1, y - 1, rect.width() + 2, rect.height() + 2, m_mask, m_maskBounds);
    fillPainter.end();

    qreal pressure = m_strengthOption.apply(info);
    quint8 *dabData = dab->data();

    KisHLineIteratorSP iter = fillDevice->createHLineIteratorNG(x, y, rect.width());
    for (int row = 0; row < rect.height(); ++row) {
        for (int col = 0; col < rect.width(); ++col) {
            if (texturingMode == MULTIPLY) {
                dab->colorSpace()->multiplyAlpha(dabData, quint8(*iter->oldRawData() * pressure), 1);
            } else {
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

