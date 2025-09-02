/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dlg_image_properties.h"

#include <QRadioButton>
#include <QLayout>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QTextEdit>

#include <klocalizedstring.h>

#include <KoColorSpace.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoColor.h>
#include <KoColorConversionTransformation.h>
#include <KoColorPopupAction.h>
#include <kis_icon_utils.h>
#include <KoID.h>
#include <kis_image.h>
#include <kis_annotation.h>
#include <kis_config.h>
#include <kis_signal_compressor.h>
#include <kis_image_config.h>
#include "widgets/kis_cmb_idlist.h"
#include <KisSqueezedComboBox.h>
#include "kis_layer_utils.h"
#include <kis_display_color_converter.h>
#include <KisWidgetConnectionUtils.h>

#include "KisProofingConfigModel.h"

struct KisDlgImageProperties::Private {
    Private(KisDisplayColorConverter *colorConverter)
        : proofingModel(KisProofingConfigModel())
        , compressor(KisSignalCompressor(500 /* ms */, KisSignalCompressor::POSTPONE))
        , colorConverter(colorConverter)
    {
    }
    KisImageWSP image;
    KisProofingConfigModel proofingModel;
    bool firstProofingConfigChange {true};
    QLabel *colorWarningLabel {0};
    KisSignalCompressor compressor ;
    KisDisplayColorConverter *colorConverter;
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

    //set the proofing space
    KisProofingConfigurationSP config = d->image->proofingConfiguration();

    if (!config) {
        config = KisImageConfig(true).defaultProofingconfiguration();
    }
    d->proofingModel.data.set(*config.data());
    connect(d->colorConverter, SIGNAL(displayConfigurationChanged()), this, SLOT(updateDisplayConfigInfo()));
    updateDisplayConfigInfo();

    m_page->gamutAlarm->setToolTip(i18n("Set color used for warning"));
    m_page->sldAdaptationState->setMaximum(d->proofingModel.adaptationRangeMax());
    m_page->sldAdaptationState->setMinimum(0);

    m_page->proofSpaceSelector->showDepth(false);

    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Perceptual"), INTENT_PERCEPTUAL);
    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Relative Colorimetric"), INTENT_RELATIVE_COLORIMETRIC);
    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Saturation"), INTENT_SATURATION);
    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Absolute Colorimetric"), INTENT_ABSOLUTE_COLORIMETRIC);

    m_page->cmbIntent->setModel(m_page->cmbDisplayIntent->model());

    m_page->cmbDisplayTransformState->addItem(i18nc("Display Mode", "Use global display settings"), int(KisProofingConfiguration::Monitor));

    m_page->cmbDisplayTransformState->addItem(i18nc("Display Mode", "Simulate paper white and black"), int(KisProofingConfiguration::Paper));
    m_page->cmbDisplayTransformState->addItem(i18nc("Display Mode", "Custom"), int(KisProofingConfiguration::Custom));

    QModelIndex idx = m_page->cmbDisplayTransformState->model()->index(m_page->cmbDisplayTransformState->findData(int(KisProofingConfiguration::Monitor)), 0);
    m_page->cmbDisplayTransformState->model()->setData(idx, i18nc("@info:tooltip", "Use Rendering Intent, Blackpoint compensation and Adaptation set in the color management configuration."), Qt::ToolTipRole);
    idx = m_page->cmbDisplayTransformState->model()->index(m_page->cmbDisplayTransformState->findData(int(KisProofingConfiguration::Paper)), 0);
    m_page->cmbDisplayTransformState->model()->setData(idx, i18nc("@info:tooltip", "Simulate paper by using absolute colorimetric and disabling white point adaptation and blackpoint compensation."), Qt::ToolTipRole);
    idx = m_page->cmbDisplayTransformState->model()->index(m_page->cmbDisplayTransformState->findData(int(KisProofingConfiguration::Custom)), 0);
    m_page->cmbDisplayTransformState->model()->setData(idx, i18nc("@info:tooltip", "Select custom settings for the second transform."), Qt::ToolTipRole);

    updateProofingWidgets();

    KisSignalCompressor *softProofConfigCompressor = new KisSignalCompressor(500, KisSignalCompressor::POSTPONE,this);

    KisWidgetConnectionUtils::connectControl(m_page->ckbBlackPointComp, &d->proofingModel, "convBlackPointCompensation");
    KisWidgetConnectionUtils::connectControl(m_page->gamutAlarm, &d->proofingModel, "warningColor");
    KisWidgetConnectionUtils::connectWidgetEnabledToProperty(m_page->grbDisplayConversion, &d->proofingModel, "enableDisplayToggles");
    KisWidgetConnectionUtils::connectWidgetEnabledToProperty(m_page->sldAdaptationState, &d->proofingModel, "enableAdaptationSlider");
    KisWidgetConnectionUtils::connectWidgetEnabledToProperty(m_page->chkDisplayBlackPointCompensation, &d->proofingModel, "enableDisplayBlackPointCompensation");

    connect(m_page->chkSaveProofing, SIGNAL(toggled(bool)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->cmbDisplayTransformState, SIGNAL(currentIndexChanged(int)), this, SLOT(proofingDisplayModeUpdated()));
    connect(m_page->proofSpaceSelector, SIGNAL(colorSpaceChanged(const KoColorSpace*)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->cmbIntent, SIGNAL(currentIndexChanged(int)), this, SLOT(proofingConversionIntentUpdated()));
    connect(m_page->cmbDisplayIntent, SIGNAL(currentIndexChanged(int)), this, SLOT(proofingDisplayIntentUpdated()));
    connect(m_page->chkDisplayBlackPointCompensation, &QCheckBox::toggled, &d->proofingModel, &KisProofingConfigModel::setdispBlackPointCompensation);
    connect(m_page->sldAdaptationState, &QSlider::valueChanged, &d->proofingModel, &KisProofingConfigModel::setadaptationState);

    connect(&d->proofingModel, SIGNAL(modelChanged()), softProofConfigCompressor, SLOT(start()));
    connect(softProofConfigCompressor, SIGNAL(timeout()), this, SLOT(setProofingConfig()));

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
    connect(this, SIGNAL(accepted()), SLOT(slotSaveDialogState()));

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

void KisDlgImageProperties::setProofingConfig()
{
    d->proofingModel.blockSignals(true);
    if (d->firstProofingConfigChange) {
        if (!d->proofingModel.storeSoftproofingInsideImage()) {
            m_page->chkSaveProofing->setChecked(true);
        }
        d->firstProofingConfigChange = false;
    }
    if (m_page->chkSaveProofing->isChecked()) {

        d->proofingModel.setproofingProfile(m_page->proofSpaceSelector->currentColorSpace()->profile()->name());
        d->proofingModel.setproofingModel(m_page->proofSpaceSelector->currentColorSpace()->colorModelId().id());
        d->proofingModel.setproofingDepth("U8");//default to this
        d->proofingModel.setstoreSoftproofingInsideImage(true);

        KisProofingConfiguration conf = d->proofingModel.data.get();
        KisProofingConfigurationSP sConf(new KisProofingConfiguration(conf));
        d->image->setProofingConfiguration(sConf);
    }
    else {
        d->image->setProofingConfiguration(KisProofingConfigurationSP());
        d->proofingModel.setstoreSoftproofingInsideImage(false);
    }
    d->proofingModel.blockSignals(false);
    updateProofingWidgets();
}

void KisDlgImageProperties::updateProofingWidgets()
{
    /// TODO: Use proper KisWidgetConnectionUtils to connect the individual
    /// controls to the model instead of updating them in bulk

    {
        QSignalBlocker b(m_page->chkSaveProofing);
        m_page->chkSaveProofing->setChecked(d->proofingModel.storeSoftproofingInsideImage());
    }

    {
        QSignalBlocker b(m_page->proofSpaceSelector);
        m_page->proofSpaceSelector->setCurrentColorSpace(
            KoColorSpaceRegistry::instance()->colorSpace(d->proofingModel.proofingModel(),
                                                         d->proofingModel.proofingDepth(),
                                                         d->proofingModel.proofingProfile()));
    }

    {
        QSignalBlocker b(m_page->cmbDisplayTransformState);
        m_page->cmbDisplayTransformState->setCurrentIndex(
            m_page->cmbDisplayTransformState->findData(int(d->proofingModel.displayTransformState()), Qt::UserRole));
    }

    {
        QSignalBlocker b(m_page->cmbIntent);
        m_page->cmbIntent->setCurrentIndex(
            m_page->cmbIntent->findData(int(d->proofingModel.conversionIntent()), Qt::UserRole));
    }
    {
        QSignalBlocker b(m_page->cmbDisplayIntent);
        m_page->cmbDisplayIntent->setCurrentIndex(
            m_page->cmbDisplayIntent->findData(int(d->proofingModel.effectiveDisplayIntent()), Qt::UserRole));
    }
    {
        QSignalBlocker b(m_page->chkDisplayBlackPointCompensation);
        m_page->chkDisplayBlackPointCompensation->setChecked(d->proofingModel.effectiveDispBlackPointCompensation());
    }

    {
        QSignalBlocker b(m_page->sldAdaptationState);
        m_page->sldAdaptationState->setValue(d->proofingModel.effectiveAdaptationState());
    }
}

void KisDlgImageProperties::updateDisplayConfigInfo()
{
    d->proofingModel.updateDisplayConfig(d->colorConverter->displayConfig());
}

void KisDlgImageProperties::proofingDisplayModeUpdated()
{
    d->proofingModel.setdisplayTransformState(KisProofingConfiguration::DisplayTransformState(m_page->cmbDisplayTransformState->currentData(Qt::UserRole).toInt()));
}

void KisDlgImageProperties::proofingConversionIntentUpdated()
{
    d->proofingModel.setconversionIntent(KoColorConversionTransformation::Intent(m_page->cmbIntent->currentData(Qt::UserRole).toInt()));
}

void KisDlgImageProperties::proofingDisplayIntentUpdated()
{
    if (d->proofingModel.displayTransformState() == KisProofingConfiguration::Custom) {
        d->proofingModel.setdisplayIntent(KoColorConversionTransformation::Intent(m_page->cmbDisplayIntent->currentData(Qt::UserRole).toInt()));
    }
}

void KisDlgImageProperties::slotSaveDialogState()
{
    setProofingConfig();

    KisConfig cfg(false);
    cfg.setConvertLayerColorSpaceInProperties(m_page->chkConvertLayers->isChecked());
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

