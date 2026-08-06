/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dlg_image_properties.h"

#include <QLabel>

#include <klocalizedstring.h>

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <kis_image.h>
#include <kis_annotation.h>
#include <kis_config.h>
#include <kis_signal_compressor.h>
#include <kis_image_config.h>
#include "kis_hdr_metadata.h"
#include "kis_layer_utils.h"
#include <kis_display_color_converter.h>
#include <KisWidgetConnectionUtils.h>

#include <kis_processing_applicator.h>
#include <kis_image_signal_router.h>
#include <kis_undo_adapter.h>
#include <kis_types.h>
#include <kis_group_layer.h>

#include <commands_new/KisChangeImageHdrContentLightLevelCommand.h>
#include <commands_new/KisChangeImageHdrColorVolumeCommand.h>
#include <commands_new/KisChangeImageHdrDiffuseWhiteCommand.h>
#include <KisContentLightLevelProcessingVistor.h>

#include "KisProofingConfigModel.h"

struct KisDlgImageProperties::Private {
    Private(KisDisplayColorConverter *colorConverter)
        : compressor(KisSignalCompressor(500 /* ms */, KisSignalCompressor::POSTPONE))
        , colorConverter(colorConverter)
        , colorVolumeCompressor(KisSignalCompressor(500 /* ms */, KisSignalCompressor::POSTPONE))
    {
    }
    KisImageWSP image;
    KisProofingConfigurationSP originalProofingConfig;
    bool firstProofingConfigChange {true};
    QLabel *colorWarningLabel {0};
    KisSignalCompressor compressor ;
    KisDisplayColorConverter *colorConverter;
    KisSignalCompressor colorVolumeCompressor;
};

KisDlgImageProperties::KisDlgImageProperties(KisImageWSP image, KisDisplayColorConverter *colorConverter, QWidget *parent, const char *name)
    : KoDialog(parent)
    , d(new Private(colorConverter))
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);
    setCaption(i18n("Image Properties"));
    m_page = new WdgImageProperties(this);

    d->image = image;

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_page->lblWidthValue->setText(QString::number(image->width()));
    m_page->lblHeightValue->setText(QString::number(image->height()));
    m_page->lblLayerCount->setText(QString::number(image->nChildLayers()));

    m_page->lblResolutionValue->setText(QLocale().toString(image->xRes()*72, 2)); // XXX: separate values for x & y?

    //Set the canvas projection color:    backgroundColor
    KoColor background = d->image->defaultProjectionColor();
    background.setOpacity(1.0);
    m_page->bnBackgroundColor->setColor(background);
    m_page->sldBackgroundColor->setRange(0.0,1.0,2);
    m_page->sldBackgroundColor->setSingleStep(0.05);
    m_page->sldBackgroundColor->setValue(d->image->defaultProjectionColor().opacityF());

    connect(m_page->bnBackgroundColor, SIGNAL(changed(KoColor)), &d->compressor, SLOT(start()));
    connect(m_page->sldBackgroundColor, SIGNAL(valueChanged(qreal)), &d->compressor, SLOT(start()));
    connect(&d->compressor, SIGNAL(timeout()), this, SLOT(setCurrentColor()));

    //Set the color space
    m_page->colorSpaceSelector->setCurrentColorSpace(image->colorSpace());
    m_page->chkConvertLayers->setChecked(KisConfig(true).convertLayerColorSpaceInProperties());

    // fetch the proofing space
    KisProofingConfigurationSP config = d->image->proofingConfiguration();
    const bool hasImageLocalConfig = bool(config);

    if (config) {
        // create a copy of the original config
        d->originalProofingConfig.reset(new KisProofingConfiguration(*config));
    } else {
        config = KisImageConfig(true).defaultProofingconfiguration();
    }

    // enable proofing config widgets only when the image has its
    // own config
    m_page->chkSaveProofing->setChecked(hasImageLocalConfig);
    connect(m_page->chkSaveProofing, &QCheckBox::toggled,
        m_page->wdgProofingOptions, &KisProofingOptionsWidget::setEnabled);
    m_page->wdgProofingOptions->setEnabled(m_page->chkSaveProofing->isChecked());

    // we should reset to the global settings when when the checkbox
    // is toggled
    connect(m_page->chkSaveProofing, &QCheckBox::toggled,
        this, &KisDlgImageProperties::setProofingConfigToImage);

    // initialize the proofing configuration widget
    m_page->wdgProofingOptions->setProofingConfig(config);
    connect(d->colorConverter, SIGNAL(displayConfigurationChanged()), this, SLOT(updateDisplayConfigInfo()));
    updateDisplayConfigInfo();

    connect(m_page->wdgProofingOptions,
            &KisProofingOptionsWidget::sigProofingConfigChanged,
            this,
            &KisDlgImageProperties::setProofingConfigToImage);

    updateHDRLightLevels();
    updateHDRColorVolume();
    connect(m_page->btnCalculateClli, &QPushButton::clicked, this, &KisDlgImageProperties::slotCalculateLightLevels);
    connect(d->image, &KisImage::sigContentLightLevelInformationChanged, this, &KisDlgImageProperties::updateHDRLightLevels);
    connect(d->image, &KisImage::sigDiffuseWhiteLightLevelChanged, this, &KisDlgImageProperties::updateHDRDiffuseWhite);
    connect(d->image, &KisImage::sigColorVolumeInformationChanged, this, &KisDlgImageProperties::updateHDRColorVolume);

    m_page->cmbDiffuseWhite->addItem(i18n("80 cd/m²"), 80.0);
    m_page->cmbDiffuseWhite->addItem(i18n("203 cd/m²"), 203.0);

    connect(m_page->gbxDiffuseWhite, &QGroupBox::clicked, this, &KisDlgImageProperties::setHDRDiffuseWhiteOnImage);
    connect(m_page->cmbDiffuseWhite, SIGNAL(activated(int)), this, SLOT(setHDRDiffuseWhiteOnImage()));

    connect(m_page->gbxContentLightLevel, &QGroupBox::clicked, this, &KisDlgImageProperties::setHDRLightLevelsOnImage);
    connect(m_page->spnMaxCll, &QDoubleSpinBox::valueChanged, this, &KisDlgImageProperties::setHDRLightLevelsOnImage);
    connect(m_page->spnMaxFall, &QDoubleSpinBox::valueChanged, this, &KisDlgImageProperties::setHDRLightLevelsOnImage);

    m_page->cmbColorVolumePresets->addItem(i18n("Rec. 2100 PQ"), "p2100-pq");
    m_page->cmbColorVolumePresets->addItem(i18n("DCI-P3 D65"), "dci-p3-d65");

    m_page->cmbLumiCalcType->addItem(i18n("XYZ Luminance"), KisRelativeContentLightLevelInformation::XYZLuminance);
    m_page->cmbLumiCalcType->addItem(i18n("Rec 2020 Per Component"), KisRelativeContentLightLevelInformation::Rec2020Component);
    m_page->cmbLumiCalcType->addItem(i18n("RGB Per Component"), KisRelativeContentLightLevelInformation::RGBComponent);

    m_page->cmbLumiCalcType->setItemData(0, i18nc("@tooltip", "Calculate the brightness in nits against XYZ luminance."), Qt::ToolTipRole);
    m_page->cmbLumiCalcType->setItemData(1, i18nc("@tooltip", "Calculate the brightness in nits by testing the components in linear rec 2020."), Qt::ToolTipRole);
    m_page->cmbLumiCalcType->setItemData(2, i18nc("@tooltip", "Calculate the brightness in nits by testing the components in linear RGB, if possible, falls back to using XYZ luminance."), Qt::ToolTipRole);

    connect(m_page->cmbColorVolumePresets, SIGNAL(activated(int)), this, SLOT(changeColorVolumePreset()));
    connect(m_page->gbxColorVolume, &QGroupBox::clicked, &d->colorVolumeCompressor, &KisSignalCompressor::start);
    connect(m_page->spnWhiteX, &QDoubleSpinBox::valueChanged, &d->colorVolumeCompressor, &KisSignalCompressor::start);
    connect(m_page->spnWhiteY, &QDoubleSpinBox::valueChanged, &d->colorVolumeCompressor, &KisSignalCompressor::start);
    connect(m_page->spnRedX, &QDoubleSpinBox::valueChanged, &d->colorVolumeCompressor, &KisSignalCompressor::start);
    connect(m_page->spnRedY, &QDoubleSpinBox::valueChanged, &d->colorVolumeCompressor, &KisSignalCompressor::start);
    connect(m_page->spnGreenX, &QDoubleSpinBox::valueChanged, &d->colorVolumeCompressor, &KisSignalCompressor::start);
    connect(m_page->spnGreenY, &QDoubleSpinBox::valueChanged, &d->colorVolumeCompressor, &KisSignalCompressor::start);
    connect(m_page->spnBlueX, &QDoubleSpinBox::valueChanged, &d->colorVolumeCompressor, &KisSignalCompressor::start);
    connect(m_page->spnBlueX, &QDoubleSpinBox::valueChanged, &d->colorVolumeCompressor, &KisSignalCompressor::start);
    connect(m_page->spnMinLuminance, &QDoubleSpinBox::valueChanged, &d->colorVolumeCompressor, &KisSignalCompressor::start);
    connect(m_page->spnMaxLuminance, &QDoubleSpinBox::valueChanged, &d->colorVolumeCompressor, &KisSignalCompressor::start);
    connect(&d->colorVolumeCompressor, &KisSignalCompressor::timeout, this, &KisDlgImageProperties::setHDRColorVolumeOnImage);

    const bool hdr = (d->image->colorSpace()->hasHighDynamicRange()
                      || d->image->colorSpace()->profile()->getTransferCharacteristics() == TRC_ITU_R_BT_2100_0_PQ);
    m_page->grpHdrMeta->setEnabled(hdr);

    //annotations
    vKisAnnotationSP_it beginIt = image->beginAnnotations();
    vKisAnnotationSP_it endIt = image->endAnnotations();

    vKisAnnotationSP_it it = beginIt;
    while (it != endIt) {

        if (!(*it) || (*it)->type().isEmpty()) {
            dbgFile << "Warning: empty annotation";
            it++;
            continue;
        }

        m_page->cmbAnnotations->addItem((*it)->type());
        it++;
    }
    connect(m_page->cmbAnnotations, SIGNAL(textActivated(QString)), SLOT(setAnnotation(QString)));
    setAnnotation(m_page->cmbAnnotations->currentText());

    connect(m_page->colorSpaceSelector,
            SIGNAL(colorSpaceChanged(const KoColorSpace*)),
            SLOT(slotColorSpaceChanged(const KoColorSpace*)));
    slotColorSpaceChanged(d->image->colorSpace());
}

KisDlgImageProperties::~KisDlgImageProperties()
{
    if (d->compressor.isActive()) {
        d->compressor.stop();
        setCurrentColor();
    }

    delete m_page;
}

int KisDlgImageProperties::exec()
{
    int resultCode = KoDialog::exec();

    m_page->wdgProofingOptions->stopPendingUpdates();

    if (resultCode == QDialog::Accepted) {
        if (m_page->chkSaveProofing->isChecked()) {
            d->image->setProofingConfiguration(m_page->wdgProofingOptions->currentProofingConfig());
        } else {
            d->image->setProofingConfiguration(nullptr);
        }
    } else {
        d->image->setProofingConfiguration(d->originalProofingConfig);
    }

    KisConfig cfg(false);
    cfg.setConvertLayerColorSpaceInProperties(m_page->chkConvertLayers->isChecked());

    return resultCode;
}

bool KisDlgImageProperties::convertLayerPixels() const
{
    return m_page->chkConvertLayers->isChecked();
}

const KoColorSpace * KisDlgImageProperties::colorSpace() const
{
    return m_page->colorSpaceSelector->currentColorSpace();
}

void KisDlgImageProperties::setCurrentColor()
{
    KoColor background = m_page->bnBackgroundColor->color();
    background.setOpacity(m_page->sldBackgroundColor->value());
    KisLayerUtils::changeImageDefaultProjectionColor(d->image, background);
}

void KisDlgImageProperties::setProofingConfigToImage()
{
    if (m_page->chkSaveProofing->isChecked()) {
        d->image->setProofingConfiguration(m_page->wdgProofingOptions->currentProofingConfig());
    } else {
        d->image->setProofingConfiguration(nullptr);
    }
}

void KisDlgImageProperties::updateDisplayConfigInfo()
{
    m_page->wdgProofingOptions->setDisplayConfigOptions(d->colorConverter->conversionOptions());
}

void KisDlgImageProperties::updateHDRDiffuseWhite()
{
    if (d->image->diffuseWhiteLightLevel()) {
        m_page->gbxDiffuseWhite->setChecked(true);
        m_page->cmbDiffuseWhite->setCurrentIndex(m_page->cmbDiffuseWhite->findData(*d->image->diffuseWhiteLightLevel()));
        if (d->image->colorSpace()->profile()->hdrReferenceWhite()) {
            m_page->gbxDiffuseWhite->setEnabled(false);
            m_page->cmbDiffuseWhite->setEnabled(false);
        }

    } else {
        m_page->gbxDiffuseWhite->setChecked(false);
        m_page->cmbDiffuseWhite->setCurrentIndex(0);
    }
}

void KisDlgImageProperties::updateHDRLightLevels()
{
    if (d->image->relativeContentLightLevelInformation()) {
        m_page->gbxContentLightLevel->setChecked(true);
        double diffuseWhite = d->image->diffuseWhiteLightLevel()? *d->image->diffuseWhiteLightLevel(): 80.0;
        KisRelativeContentLightLevelInformation clli = *d->image->relativeContentLightLevelInformation();
        m_page->spnMaxCll->setValue(clli.maxContentLightLevel*diffuseWhite);
        m_page->spnMaxFall->setValue(clli.maxFrameAverageLightLevel*diffuseWhite);
        m_page->cmbLumiCalcType->setCurrentIndex(clli.type);
    } else {
        m_page->gbxContentLightLevel->setChecked(false);
    }
}

void KisDlgImageProperties::updateHDRColorVolume()
{
    if (d->image->colorVolumeInformation()) {
        m_page->gbxColorVolume->setChecked(true);
        KisColorVolumeInformation cvi = *d->image->colorVolumeInformation();
        m_page->spnWhiteX->setValue(cvi.white.x());
        m_page->spnWhiteY->setValue(cvi.white.y());

        m_page->spnRedX->setValue(cvi.red.x());
        m_page->spnRedY->setValue(cvi.red.y());

        m_page->spnGreenX->setValue(cvi.green.x());
        m_page->spnGreenY->setValue(cvi.green.y());

        m_page->spnBlueX->setValue(cvi.blue.x());
        m_page->spnBlueY->setValue(cvi.blue.y());

        m_page->spnMaxLuminance->setValue(cvi.maxLuminance);
        m_page->spnMinLuminance->setValue(cvi.minLuminance);

    } else {
        m_page->gbxColorVolume->setChecked(false);
    }
}

void KisDlgImageProperties::setHDRLightLevelsOnImage()
{
    std::optional<KisRelativeContentLightLevelInformation> optClli = std::nullopt;

    if (m_page->gbxContentLightLevel->isChecked()) {
        double diffuseWhite = d->image->diffuseWhiteLightLevel().value_or(80.0);
        KisRelativeContentLightLevelInformation clli;
        clli.maxContentLightLevel = m_page->spnMaxCll->value() / diffuseWhite;
        clli.maxFrameAverageLightLevel = m_page->spnMaxFall->value() / diffuseWhite;
        clli.type = KisRelativeContentLightLevelInformation::CalculationType(m_page->cmbLumiCalcType->currentData().toInt());
        optClli = std::make_optional(clli);
    }
    if (d->image->relativeContentLightLevelInformation() != optClli) {
        KUndo2Command *cmd = new KisChangeImageHdrContentLightLevelCommand(d->image, optClli);
        d->image->undoAdapter()->addCommand(cmd);
    }
}

void KisDlgImageProperties::setHDRDiffuseWhiteOnImage()
{
    std::optional<double> dw = std::nullopt;
    if (m_page->gbxDiffuseWhite->isChecked()) {
        dw = std::make_optional(m_page->cmbDiffuseWhite->currentData().toDouble());
    }
    if (d->image->diffuseWhiteLightLevel() != dw) {
        KUndo2Command *cmd = new KisChangeImageHdrDiffuseWhiteCommand(d->image, dw);
        d->image->undoAdapter()->addCommand(cmd);
    }
}

void KisDlgImageProperties::setHDRColorVolumeOnImage()
{
    std::optional<KisColorVolumeInformation> optCvi = std::nullopt;
    if (m_page->gbxColorVolume->isChecked()) {
        KisColorVolumeInformation cvi;

        cvi.white = QPointF(m_page->spnWhiteX->value(), m_page->spnWhiteY->value());

        cvi.red = QPointF(m_page->spnRedX->value(), m_page->spnRedY->value());
        cvi.green = QPointF(m_page->spnGreenX->value(), m_page->spnGreenY->value());
        cvi.blue = QPointF(m_page->spnBlueX->value(), m_page->spnBlueY->value());

        cvi.maxLuminance = m_page->spnMaxLuminance->value();
        cvi.minLuminance = m_page->spnMinLuminance->value();

        optCvi = std::make_optional(cvi);

    }
    if (d->image->colorVolumeInformation() != optCvi) {
        KUndo2Command *cmd = new KisChangeImageHdrColorVolumeCommand(d->image, optCvi);
        d->image->undoAdapter()->addCommand(cmd);
    }
}

void KisDlgImageProperties::changeColorVolumePreset()
{
    const QString displayId = m_page->cmbColorVolumePresets->currentData().toString();

    if (displayId == "p2100-pq") {
        m_page->spnRedX->setValue(0.708);
        m_page->spnRedY->setValue(0.292);

        m_page->spnGreenX->setValue(0.170);
        m_page->spnGreenY->setValue(0.797);

        m_page->spnBlueX->setValue(0.131);
        m_page->spnBlueY->setValue(0.046);

        m_page->spnWhiteX->setValue(0.3127);
        m_page->spnWhiteY->setValue(0.3290);

        m_page->spnMinLuminance->setValue(0.005);
        m_page->spnMaxLuminance->setValue(1000);

    } else if (displayId == "dci-p3-d65") {

        m_page->spnRedX->setValue(0.680);
        m_page->spnRedY->setValue(0.320);

        m_page->spnGreenX->setValue(0.265);
        m_page->spnGreenY->setValue(0.690);

        m_page->spnBlueX->setValue(0.150);
        m_page->spnBlueY->setValue(0.060);

        m_page->spnWhiteX->setValue(0.3127);
        m_page->spnWhiteY->setValue(0.3290);

        m_page->spnMinLuminance->setValue(0.005);
        m_page->spnMaxLuminance->setValue(1000);

    }
}


void KisDlgImageProperties::slotCalculateLightLevels()
{
    KisRelativeContentLightLevelInformation::CalculationType type = KisRelativeContentLightLevelInformation::CalculationType(m_page->cmbLumiCalcType->currentData().toInt());

    KisProcessingApplicator::ProcessingFlags signalFlags = KisProcessingApplicator::NO_UI_UPDATES | KisProcessingApplicator::RECURSIVE_FRAME_TIMES;
    KisProcessingApplicator applicator(d->image, d->image->rootLayer(),
                                       signalFlags);

    KisSharedPtr<KisContentLightLevelProcessingVistor> visitor =
        new KisContentLightLevelProcessingVistor(type, d->image->bounds());

    applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::SEQUENTIAL);

    auto cmd = new KisCommandUtils::LambdaCommand(
        [visitor, image = d->image] () {
            auto info = visitor->contentLightLevelInformation();
            return new KisChangeImageHdrContentLightLevelCommand(image, info);
        });
    applicator.applyCommand(cmd, KisStrokeJobData::SEQUENTIAL);

    applicator.end();
}

void KisDlgImageProperties::slotColorSpaceChanged(const KoColorSpace *cs)
{
    if (*d->image->profile() != *cs->profile() &&
        !KisLayerUtils::canChangeImageProfileInvisibly(d->image)) {

        m_page->wdgWarningNotice->setVisible(true);
        m_page->wdgWarningNotice->setText(
                    m_page->wdgWarningNotice->changeImageProfileWarningText());
    } else {
        m_page->wdgWarningNotice->setVisible(false);
    }
}

void KisDlgImageProperties::setAnnotation(const QString &type)
{
    KisAnnotationSP annotation = d->image->annotation(type);
    if (annotation) {
        m_page->lblDescription->clear();
        m_page->txtAnnotation->clear();
        m_page->lblDescription->setText(annotation->description());
        m_page->txtAnnotation->appendPlainText(annotation->displayText());
    }
    else {
        m_page->lblDescription->clear();
        m_page->txtAnnotation->clear();
    }
}

