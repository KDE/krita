/*
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
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

    m_page->lblResolutionValue->setText(QLocale().toString(image->xRes()*72, 2)); // XXX: separate values for x & y?

    //Set the canvas projection color:    backgroundColor
    KoColor background = m_image->defaultProjectionColor();
    background.setOpacity(1.0);
    m_page->bnBackgroundColor->setColor(background);
    m_page->sldBackgroundColor->setRange(0.0,1.0,2);
    m_page->sldBackgroundColor->setSingleStep(0.05);
    m_page->sldBackgroundColor->setValue(m_image->defaultProjectionColor().opacityF());

    KisSignalCompressor *compressor = new KisSignalCompressor(500 /* ms */, KisSignalCompressor::POSTPONE, this);
    connect(m_page->bnBackgroundColor, SIGNAL(changed(KoColor)), compressor, SLOT(start()));
    connect(m_page->sldBackgroundColor, SIGNAL(valueChanged(qreal)), compressor, SLOT(start()));
    connect(compressor, SIGNAL(timeout()), this, SLOT(setCurrentColor()));

    //Set the color space
    m_page->colorSpaceSelector->setCurrentColorSpace(image->colorSpace());

    //set the proofing space
    m_proofingConfig = m_image->proofingConfiguration();
    if (!m_proofingConfig) {
        m_page->chkSaveProofing->setChecked(false);
        m_proofingConfig = KisImageConfig(true).defaultProofingconfiguration();
    }
    else {
        m_page->chkSaveProofing->setChecked(true);
    }

    m_page->proofSpaceSelector->setCurrentColorSpace(KoColorSpaceRegistry::instance()->colorSpace(m_proofingConfig->proofingModel, m_proofingConfig->proofingDepth, m_proofingConfig->proofingProfile));
    m_page->cmbIntent->setCurrentIndex((int)m_proofingConfig->intent);
    m_page->ckbBlackPointComp->setChecked(m_proofingConfig->conversionFlags.testFlag(KoColorConversionTransformation::BlackpointCompensation));

    m_page->gamutAlarm->setColor(m_proofingConfig->warningColor);
    m_page->gamutAlarm->setToolTip(i18n("Set color used for warning"));
    m_page->sldAdaptationState->setMaximum(20);
    m_page->sldAdaptationState->setMinimum(0);
    m_page->sldAdaptationState->setValue((int)m_proofingConfig->adaptationState*20);

    KisSignalCompressor *softProofConfigCompressor = new KisSignalCompressor(500, KisSignalCompressor::POSTPONE,this);

    connect(m_page->gamutAlarm, SIGNAL(changed(KoColor)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->proofSpaceSelector, SIGNAL(colorSpaceChanged(const KoColorSpace*)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->cmbIntent, SIGNAL(currentIndexChanged(int)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->ckbBlackPointComp, SIGNAL(stateChanged(int)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->sldAdaptationState, SIGNAL(valueChanged(int)), softProofConfigCompressor, SLOT(start()));

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

        m_page->cmbAnnotations->addItem((*it) -> type());
        it++;
    }
    connect(m_page->cmbAnnotations, SIGNAL(activated(QString)), SLOT(setAnnotation(QString)));
    setAnnotation(m_page->cmbAnnotations->currentText());

}

KisDlgImageProperties::~KisDlgImageProperties()
{
    delete m_page;
}

const KoColorSpace * KisDlgImageProperties::colorSpace()
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
    if (m_firstProofingConfigChange) {
        m_page->chkSaveProofing->setChecked(true);
        m_firstProofingConfigChange = false;
    }
    if (m_page->chkSaveProofing->isChecked()) {

        m_proofingConfig->conversionFlags = KoColorConversionTransformation::HighQuality;

#if QT_VERSION >= 0x050700
        m_proofingConfig->conversionFlags.setFlag(KoColorConversionTransformation::BlackpointCompensation, m_page->ckbBlackPointComp->isChecked());
#else
        m_page->ckbBlackPointComp->isChecked() ?
            m_proofingConfig->conversionFlags |= KoColorConversionTransformation::BlackpointCompensation
            : m_proofingConfig->conversionFlags = m_proofingConfig->conversionFlags & ~KoColorConversionTransformation::BlackpointCompensation;

#endif
        m_proofingConfig->intent = (KoColorConversionTransformation::Intent)m_page->cmbIntent->currentIndex();
        m_proofingConfig->proofingProfile = m_page->proofSpaceSelector->currentColorSpace()->profile()->name();
        m_proofingConfig->proofingModel = m_page->proofSpaceSelector->currentColorSpace()->colorModelId().id();
        m_proofingConfig->proofingDepth = "U8";//default to this
        m_proofingConfig->warningColor = m_page->gamutAlarm->color();
        m_proofingConfig->adaptationState = (double)m_page->sldAdaptationState->value()/20.0;

        m_image->setProofingConfiguration(m_proofingConfig);
    }
    else {
        m_image->setProofingConfiguration(KisProofingConfigurationSP());
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

