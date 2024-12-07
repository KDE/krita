/*
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_dlg_image_properties.h"

#include <QPushButton>
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


KisDlgImageProperties::KisDlgImageProperties(KisImageWSP image, QWidget *parent, const char *name)
    : KoDialog(parent)
    , m_proofingModel(new KisProofingConfigModel())
{
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);
    setObjectName(name);
    setCaption(i18n("Image Properties"));
    m_page = new WdgImageProperties(this);

    m_image = image;

    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_page->lblWidthValue->setText(QString::number(image->width()));
    m_page->lblHeightValue->setText(QString::number(image->height()));
    m_page->lblLayerCount->setText(QString::number(image->nChildLayers()));

    m_page->lblResolutionValue->setText(QLocale().toString(image->xRes()*72, 2)); // XXX: separate values for x & y?

    //Set the canvas projection color:    backgroundColor
    KoColor background = m_image->defaultProjectionColor();
    background.setOpacity(1.0);
    m_page->bnBackgroundColor->setColor(background);
    m_page->sldBackgroundColor->setRange(0.0,1.0,2);
    m_page->sldBackgroundColor->setSingleStep(0.05);
    m_page->sldBackgroundColor->setValue(m_image->defaultProjectionColor().opacityF());

    m_compressor = new KisSignalCompressor(500 /* ms */, KisSignalCompressor::POSTPONE, this);
    connect(m_page->bnBackgroundColor, SIGNAL(changed(KoColor)), m_compressor, SLOT(start()));
    connect(m_page->sldBackgroundColor, SIGNAL(valueChanged(qreal)), m_compressor, SLOT(start()));
    connect(m_compressor, SIGNAL(timeout()), this, SLOT(setCurrentColor()));

    //Set the color space
    m_page->colorSpaceSelector->setCurrentColorSpace(image->colorSpace());
    m_page->chkConvertLayers->setChecked(KisConfig(true).convertLayerColorSpaceInProperties());

    //set the proofing space
    KisProofingConfigurationSP config = m_image->proofingConfiguration();

    if (!config) {
        config = KisImageConfig(true).defaultProofingconfiguration();
    }
    m_proofingModel->data.set(*config.data());


    m_page->gamutAlarm->setToolTip(i18n("Set color used for warning"));
    m_page->sldAdaptationState->setMaximum(m_proofingModel->adaptationRangeMax());
    m_page->sldAdaptationState->setMinimum(0);

    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Perceptual"), INTENT_PERCEPTUAL);
    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Relative Colorimetric"), INTENT_RELATIVE_COLORIMETRIC);
    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Saturation"), INTENT_SATURATION);
    m_page->cmbDisplayIntent->addItem(i18nc("Color conversion intent", "Absolute Colorimetric"), INTENT_ABSOLUTE_COLORIMETRIC);

    m_page->cmbIntent->setModel(m_page->cmbDisplayIntent->model());

    m_page->cmbDisplayTransformState->addItem(i18nc("Display Mode", "Monitor"), int(KisProofingConfigModel::Monitor));

    m_page->cmbDisplayTransformState->addItem(i18nc("Display Mode", "Paper"), int(KisProofingConfigModel::Paper));
    m_page->cmbDisplayTransformState->addItem(i18nc("Display Mode", "Custom"), int(KisProofingConfigModel::Custom));

    QModelIndex idx = m_page->cmbDisplayTransformState->model()->index(m_page->cmbDisplayTransformState->findData(int(KisProofingConfigModel::Monitor)), 0);
    m_page->cmbDisplayTransformState->model()->setData(idx, i18nc("@info:tooltip", "Use Rendering Intent, Blackpoint compensation and Adaptation set in the color management configuration."), Qt::ToolTipRole);
    idx = m_page->cmbDisplayTransformState->model()->index(m_page->cmbDisplayTransformState->findData(int(KisProofingConfigModel::Paper)), 0);
    m_page->cmbDisplayTransformState->model()->setData(idx, i18nc("@info:tooltip", "Simulate paper by using absolute colorimetric and disabling white point adaptation and blackpoint compensation."), Qt::ToolTipRole);
    idx = m_page->cmbDisplayTransformState->model()->index(m_page->cmbDisplayTransformState->findData(int(KisProofingConfigModel::Custom)), 0);
    m_page->cmbDisplayTransformState->model()->setData(idx, i18nc("@info:tooltip", "Select custom settings for the second transform."), Qt::ToolTipRole);

    updateProofingWidgets();

    KisSignalCompressor *softProofConfigCompressor = new KisSignalCompressor(500, KisSignalCompressor::POSTPONE,this);

    connect(m_page->chkSaveProofing, SIGNAL(toggled(bool)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->cmbDisplayTransformState, SIGNAL(currentIndexChanged(int)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->chkDisplayBlackPointCompensation, SIGNAL(toggled(bool)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->gamutAlarm, SIGNAL(changed(KoColor)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->proofSpaceSelector, SIGNAL(colorSpaceChanged(const KoColorSpace*)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->cmbIntent, SIGNAL(currentIndexChanged(int)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->cmbDisplayIntent, SIGNAL(currentIndexChanged(int)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->ckbBlackPointComp, SIGNAL(stateChanged(int)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->sldAdaptationState, SIGNAL(valueChanged(int)), softProofConfigCompressor, SLOT(start()));

    connect(m_proofingModel, SIGNAL(modelChanged()), this, SLOT(updateProofingWidgets()));
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
    connect(m_page->cmbAnnotations, SIGNAL(activated(QString)), SLOT(setAnnotation(QString)));
    setAnnotation(m_page->cmbAnnotations->currentText());
    connect(this, SIGNAL(accepted()), SLOT(slotSaveDialogState()));

    connect(m_page->colorSpaceSelector,
            SIGNAL(colorSpaceChanged(const KoColorSpace*)),
            SLOT(slotColorSpaceChanged(const KoColorSpace*)));

    slotColorSpaceChanged(m_image->colorSpace());
}

KisDlgImageProperties::~KisDlgImageProperties()
{
    if (m_compressor->isActive()) {
        m_compressor->stop();
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
    KisLayerUtils::changeImageDefaultProjectionColor(m_image, background);
}

void KisDlgImageProperties::setProofingConfig()
{
    m_proofingModel->blockSignals(true);
    if (m_firstProofingConfigChange) {
        if (!m_proofingModel->storeSoftproofingInsideImage()) {
            m_page->chkSaveProofing->setChecked(true);
        }
        m_firstProofingConfigChange = false;
    }
    if (m_page->chkSaveProofing->isChecked()) {
        m_proofingModel->setdisplayTransformState(KisProofingConfigModel::DisplayTransformState(m_page->cmbDisplayTransformState->currentData(Qt::UserRole).toInt()));
        if (m_proofingModel->displayTransformState() == KisProofingConfigModel::Custom) {
            m_proofingModel->setdisplayIntent(KoColorConversionTransformation::Intent(m_page->cmbDisplayIntent->currentData(Qt::UserRole).toInt()));
            m_proofingModel->setadaptationState((double)m_page->sldAdaptationState->value());
            m_proofingModel->setdispBlackPointCompensation(m_page->chkDisplayBlackPointCompensation->isChecked());
        }

        m_proofingModel->setconversionIntent(KoColorConversionTransformation::Intent(m_page->cmbIntent->currentData(Qt::UserRole).toInt()));
        m_proofingModel->setconvBlackPointCompensation(m_page->ckbBlackPointComp->isChecked());
        m_proofingModel->setproofingProfile(m_page->proofSpaceSelector->currentColorSpace()->profile()->name());
        m_proofingModel->setproofingModel(m_page->proofSpaceSelector->currentColorSpace()->colorModelId().id());
        m_proofingModel->setproofingDepth("U8");//default to this
        m_proofingModel->setwarningColor(m_page->gamutAlarm->color());
        m_proofingModel->setstoreSoftproofingInsideImage(true);

        KisProofingConfiguration conf = m_proofingModel->data.get();
        KisProofingConfigurationSP sConf(new KisProofingConfiguration(conf));
        m_image->setProofingConfiguration(sConf);
    }
    else {
        m_image->setProofingConfiguration(KisProofingConfigurationSP());
        m_proofingModel->setstoreSoftproofingInsideImage(false);
    }
    m_proofingModel->blockSignals(false);
    updateProofingWidgets();
}

void KisDlgImageProperties::updateProofingWidgets()
{
    m_page->chkSaveProofing->setChecked(m_proofingModel->storeSoftproofingInsideImage());

    m_page->proofSpaceSelector->setCurrentColorSpace(KoColorSpaceRegistry::instance()->colorSpace(m_proofingModel->proofingModel(), m_proofingModel->proofingDepth(), m_proofingModel->proofingProfile()));

    m_page->ckbBlackPointComp->setChecked(m_proofingModel->convBlackPointCompensation());
    m_page->chkDisplayBlackPointCompensation->setChecked(m_proofingModel->dispBlackPointCompensation());

    m_page->sldAdaptationState->setValue((int)m_proofingModel->adaptationState());
    m_page->gamutAlarm->setColor(m_proofingModel->warningColor());

    m_page->cmbDisplayTransformState->setCurrentIndex(m_page->cmbDisplayTransformState->findData(int(m_proofingModel->displayTransformState()), Qt::UserRole));

    m_page->grbDisplayConversion->setEnabled(m_proofingModel->displayTransformState() == KisProofingConfigModel::Custom);
    m_page->sldAdaptationState->setEnabled(m_proofingModel->displayIntent() == KoColorConversionTransformation::IntentAbsoluteColorimetric);
    m_page->chkDisplayBlackPointCompensation->setEnabled(m_proofingModel->displayIntent() != KoColorConversionTransformation::IntentAbsoluteColorimetric);

    m_page->cmbIntent->setCurrentIndex(m_page->cmbIntent->findData(int(m_proofingModel->conversionIntent()), Qt::UserRole));
    m_page->cmbDisplayIntent->setCurrentIndex(m_page->cmbDisplayIntent->findData(int(m_proofingModel->displayIntent()), Qt::UserRole));
}

void KisDlgImageProperties::slotSaveDialogState()
{
    setProofingConfig();

    KisConfig cfg(false);
    cfg.setConvertLayerColorSpaceInProperties(m_page->chkConvertLayers->isChecked());
}

void KisDlgImageProperties::slotColorSpaceChanged(const KoColorSpace *cs)
{
    if (*m_image->profile() != *cs->profile() &&
        !KisLayerUtils::canChangeImageProfileInvisibly(m_image)) {

        m_page->wdgWarningNotice->setVisible(true);
        m_page->wdgWarningNotice->setText(
                    m_page->wdgWarningNotice->changeImageProfileWarningText());
    } else {
        m_page->wdgWarningNotice->setVisible(false);
    }
}

void KisDlgImageProperties::setAnnotation(const QString &type)
{
    KisAnnotationSP annotation = m_image->annotation(type);
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

