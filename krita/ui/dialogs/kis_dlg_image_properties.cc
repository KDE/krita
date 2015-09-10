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
#include <kglobal.h>

#include <KoColorSpace.h>
#include "KoColorProfile.h"
#include "KoColor.h"
#include "KoColorPopupAction.h"
#include "KoIcon.h"
#include "KoID.h"
#include "kis_types.h"
#include "kis_image.h"
#include "kis_annotation.h"
#include "kis_config.h"
#include "kis_signal_compressor.h"
#include "widgets/kis_cmb_idlist.h"
#include "widgets/squeezedcombobox.h"

KisDlgImageProperties::KisDlgImageProperties(KisImageWSP image, QWidget *parent, const char *name)
    : KDialog(parent)
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

    m_page->lblResolutionValue->setText(KGlobal::locale()->formatNumber(image->xRes()*72, 2)); // XXX: separate values for x & y?

    m_defaultColorAction = new KoColorPopupAction(this);
    m_defaultColorAction->setCurrentColor(m_image->defaultProjectionColor());
    m_defaultColorAction->setIcon(koIcon("format-stroke-color"));
    m_defaultColorAction->setToolTip(i18n("Change the background color of the image"));
    m_page->bnBackgroundColor->setDefaultAction(m_defaultColorAction);

    KisSignalCompressor *compressor = new KisSignalCompressor(500 /* ms */, KisSignalCompressor::POSTPONE, this);
    connect(m_defaultColorAction, SIGNAL(colorChanged(const KoColor&)), compressor, SLOT(start()));
    connect(compressor, SIGNAL(timeout()), this, SLOT(setCurrentColor()));

    connect(m_defaultColorAction, SIGNAL(colorChanged(const KoColor&)), this, SLOT(setCurrentColor()));

    m_page->colorSpaceSelector->setCurrentColorSpace(image->colorSpace());

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
    m_image->setDefaultProjectionColor(m_defaultColorAction->currentKoColor());
    m_image->refreshGraphAsync();
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

