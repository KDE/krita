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
#include "KoColorProfile.h"
#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoColorConversionTransformation.h"
#include "KoColorPopupAction.h"
#include "kis_icon_utils.h"
#include "KoID.h"
#include "kis_image.h"
#include "kis_annotation.h"
#include "kis_config.h"
#include "kis_signal_compressor.h"
#include "widgets/kis_cmb_idlist.h"
#include "widgets/squeezedcombobox.h"
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

    KisConfig cfg;

    m_page->lblWidthValue->setText(QString::number(image->width()));
    m_page->lblHeightValue->setText(QString::number(image->height()));

    m_page->lblResolutionValue->setText(QLocale().toString(image->xRes()*72, 2)); // XXX: separate values for x & y?

    //Set the canvas projection color:
    m_defaultColorAction = new KoColorPopupAction(this);
    m_defaultColorAction->setCurrentColor(m_image->defaultProjectionColor());
    m_defaultColorAction->setToolTip(i18n("Change the background color of the image"));
    m_page->bnBackgroundColor->setDefaultAction(m_defaultColorAction);

    KisSignalCompressor *compressor = new KisSignalCompressor(500 /* ms */, KisSignalCompressor::POSTPONE, this);
    connect(m_defaultColorAction, SIGNAL(colorChanged(const KoColor&)), compressor, SLOT(start()));
    connect(compressor, SIGNAL(timeout()), this, SLOT(setCurrentColor()));

    connect(m_defaultColorAction, SIGNAL(colorChanged(const KoColor&)), this, SLOT(setCurrentColor()));

    //Set the color space
    m_page->colorSpaceSelector->setCurrentColorSpace(image->colorSpace());

    //set the proofing space
    m_proofingConfig = m_image->proofingConfiguration();
    m_page->proofSpaceSelector->setCurrentColorSpace(KoColorSpaceRegistry::instance()->colorSpace(m_proofingConfig->proofingModel, m_proofingConfig->proofingDepth,m_proofingConfig->proofingProfile));

    m_page->cmbIntent->setCurrentIndex((int)m_proofingConfig->intent);

    m_page->ckbBlackPointComp->setChecked(m_proofingConfig->conversionFlags.testFlag(KoColorConversionTransformation::BlackpointCompensation));

    m_gamutWarning = new KoColorPopupAction(this);
    m_gamutWarning->setCurrentColor(m_proofingConfig->warningColor);
    m_gamutWarning->setToolTip(i18n("Set color used for warning"));
    m_page->gamutAlarm->setDefaultAction(m_gamutWarning);

    KisSignalCompressor *softProofConfigCompressor = new KisSignalCompressor(500, KisSignalCompressor::POSTPONE,this);

    connect(m_gamutWarning, SIGNAL(colorChanged(KoColor)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->proofSpaceSelector, SIGNAL(colorSpaceChanged(const KoColorSpace*)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->cmbIntent, SIGNAL(currentIndexChanged(int)), softProofConfigCompressor, SLOT(start()));
    connect(m_page->ckbBlackPointComp, SIGNAL(stateChanged(int)), softProofConfigCompressor, SLOT(start()));

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
    KisLayerUtils::changeImageDefaultProjectionColor(m_image, m_defaultColorAction->currentKoColor());
}

void KisDlgImageProperties::setProofingConfig()
{
    m_proofingConfig->conversionFlags = KoColorConversionTransformation::HighQuality;
    if (m_page->ckbBlackPointComp) m_proofingConfig->conversionFlags |= KoColorConversionTransformation::BlackpointCompensation;
    m_proofingConfig->intent = (KoColorConversionTransformation::Intent)m_page->cmbIntent->currentIndex();
    m_proofingConfig->proofingProfile = m_page->proofSpaceSelector->currentColorSpace()->profile()->name();
    m_proofingConfig->proofingModel = m_page->proofSpaceSelector->currentColorSpace()->colorModelId().id();
    m_proofingConfig->proofingDepth = "U8";//default to this
    m_proofingConfig->warningColor = m_gamutWarning->currentKoColor();

    qDebug()<<"set proofing config in properties: "<<m_proofingConfig->proofingProfile;
    m_image->setProofingConfiguration(m_proofingConfig);
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

