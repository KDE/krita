/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dlg_image_properties.h"

#include <QLabel>
#include <QPushButton>

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
#include <kis_hdr_metadata_models.h>
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

#include <kis_signal_compressor_with_param.h>

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
    KisHDRMetadataModel hdrMetadataModel;
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

    updateHDRDiffuseWhite();
    d->hdrMetadataModel.setClli(d->image->relativeContentLightLevelInformation());
    d->hdrMetadataModel.setCvi(d->image->colorVolumeInformation());
    connect(m_page->btnCalculateClli, &QPushButton::clicked, this, &KisDlgImageProperties::slotCalculateLightLevels);
    connect(d->image, &KisImage::sigColorSpaceChanged, this, &KisDlgImageProperties::updateHDRDiffuseWhite);
    connect(d->image, &KisImage::sigContentLightLevelInformationChanged, this, &KisDlgImageProperties::updateHDRLightLevels);
    connect(d->image, &KisImage::sigDiffuseWhiteLightLevelChanged, this, &KisDlgImageProperties::updateHDRDiffuseWhite);
    connect(d->image, &KisImage::sigColorVolumeInformationChanged, this, &KisDlgImageProperties::updateHDRColorVolume);

    KisWidgetConnectionUtils::connectControlState( m_page->cmbDiffuseWhite,  &d->hdrMetadataModel, "referenceWhiteState", "referenceWhiteIndex");

    KisWidgetConnectionUtils::connectControlState( m_page->gbxDiffuseWhite,  &d->hdrMetadataModel, "referenceWhitePresentState", "referenceWhitePresent");

    KisWidgetConnectionUtils::connectControlState( m_page->gbxContentLightLevel,  &d->hdrMetadataModel, "clliPresentState", "clliPresent");
    KisWidgetConnectionUtils::connectControl( m_page->spnMaxCll,  &d->hdrMetadataModel, "maxContentLightLevel");
    KisWidgetConnectionUtils::connectControl( m_page->spnMaxFall,  &d->hdrMetadataModel, "maxFrameAverageLightLevel");
    KisWidgetConnectionUtils::connectControlState( m_page->cmbLumiCalcType,  &d->hdrMetadataModel, "clliCalculationTypeState", "clliCalculationType");

    KisWidgetConnectionUtils::connectControl( m_page->gbxColorVolume, &d->hdrMetadataModel, "cviPresent");

    KisWidgetConnectionUtils::connectControl( m_page->spnWhiteX, &d->hdrMetadataModel, "cviWhiteX");
    KisWidgetConnectionUtils::connectControl( m_page->spnWhiteY, &d->hdrMetadataModel, "cviWhiteY");
    KisWidgetConnectionUtils::connectControl( m_page->spnRedX, &d->hdrMetadataModel, "cviRedX");
    KisWidgetConnectionUtils::connectControl( m_page->spnRedY, &d->hdrMetadataModel, "cviRedY");
    KisWidgetConnectionUtils::connectControl( m_page->spnGreenX, &d->hdrMetadataModel, "cviGreenX");
    KisWidgetConnectionUtils::connectControl( m_page->spnGreenY, &d->hdrMetadataModel, "cviGreenY");
    KisWidgetConnectionUtils::connectControl( m_page->spnBlueX, &d->hdrMetadataModel, "cviBlueX");
    KisWidgetConnectionUtils::connectControl( m_page->spnBlueY, &d->hdrMetadataModel, "cviBlueY");
    KisWidgetConnectionUtils::connectControl( m_page->spnMinLuminance, &d->hdrMetadataModel, "cviMinLuminance");
    KisWidgetConnectionUtils::connectControl( m_page->spnMaxLuminance, &d->hdrMetadataModel, "cviMaxLuminance");

    KisWidgetConnectionUtils::connectControlState( m_page->cmbColorVolumePresets, &d->hdrMetadataModel, "cviEffectivePresetIndexState", "cviEffectivePresetIndex");

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
        setHDRDiffuseWhiteOnImage();
        setHDRLightLevelsOnImage();
        setHDRColorVolumeOnImage();
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
    d->hdrMetadataModel.setImageProfileHdrReferenceWhite(d->image->colorSpace()->profile()->hdrReferenceWhite());
    d->hdrMetadataModel.setImageHdrReferenceWhiteMetadata(d->image->hdrReferenceWhiteLightLevelMetadata());
}

void KisDlgImageProperties::updateHDRLightLevels()
{
    d->hdrMetadataModel.setClli(d->image->relativeContentLightLevelInformation());
}

void KisDlgImageProperties::updateHDRColorVolume()
{
    d->hdrMetadataModel.setCvi(d->image->colorVolumeInformation());
}

void KisDlgImageProperties::setHDRLightLevelsOnImage()
{
    std::optional<KisRelativeContentLightLevelInformation> optClli = d->hdrMetadataModel.clli();

    if (d->image->relativeContentLightLevelInformation() != optClli) {
        KisProcessingApplicator::runSingleCommandStroke(d->image,
            new KisChangeImageHdrContentLightLevelCommand(d->image, optClli));
    }
}

void KisDlgImageProperties::setHDRDiffuseWhiteOnImage()
{
    std::optional<double> dw = d->hdrMetadataModel.imageHdrReferenceWhiteMetadata();
    if (d->image->hdrReferenceWhiteLightLevelMetadata() != dw) {
        KUndo2Command *cmd = new KisChangeImageHdrDiffuseWhiteCommand(d->image, dw);
        d->image->undoAdapter()->addCommand(cmd);
    }
}

void KisDlgImageProperties::setHDRColorVolumeOnImage()
{
    std::optional<KisColorVolumeInformation> optCvi = d->hdrMetadataModel.cvi();
    if (d->image->colorVolumeInformation() != optCvi) {
        KUndo2Command *cmd = new KisChangeImageHdrColorVolumeCommand(d->image, optCvi);
        d->image->undoAdapter()->addCommand(cmd);
    }
}

void KisDlgImageProperties::slotCalculateLightLevels()
{
    KisRelativeContentLightLevelInformation::CalculationType type = d->hdrMetadataModel.clliCalculationType();
    KisSharedPtr<KisContentLightLevelProcessingVistor> visitor =
            new KisContentLightLevelProcessingVistor(type, d->image->bounds());

    KisProcessingApplicator::ProcessingFlags flags =
        KisProcessingApplicator::RECURSIVE_FRAME_TIMES | KisProcessingApplicator::READ_ONLY_ACTION;
    KisProcessingApplicator applicator(d->image, d->image->rootLayer(), flags);

    applicator.applyVisitorAllFrames(visitor);

    // a poor-man implementation of a QFuture (which is Qt6-only),
    // the proxy object will notify us manually when the visitor
    // has completed its execution
    QSharedPointer<FunctionToSignalProxy> completionSignalProxy(new FunctionToSignalProxy);
    connect(completionSignalProxy.get(), &FunctionToSignalProxy::timeout, this,
        [visitor, model = &d->hdrMetadataModel, image = d->image]() {
            auto info = visitor->contentLightLevelInformation();

            model->setClli(info);
        });

    // add the command **after** the signal has been set up; this
    // is not a QFuture, so it cannot store the actual "finished"
    // state
    applicator.applyCommand(new KisCommandUtils::LambdaCommand([completionSignalProxy]() {
        completionSignalProxy->start();
        return nullptr;
    }));

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

