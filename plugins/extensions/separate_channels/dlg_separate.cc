/*
 *  dlg_separate.cc - part of KimageShop^WKrayon^WKrita
 *
 *  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "dlg_separate.h"

#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>

#include <KisDialogStateSaver.h>

#include <klocalizedstring.h>
#include <kis_debug.h>
DlgSeparate::DlgSeparate(const QString & imageCS,
                         const QString & layerCS,
                         QWidget *  parent,
                         const char * name)
        : KoDialog(parent)
        , m_imageCS(imageCS)
        , m_layerCS(layerCS)
{
    setObjectName(name);
    setCaption(i18n("Separate Image"));
    setButtons(Ok | Cancel);
    setDefaultButton(Ok);

    m_page = new WdgSeparations(this);
    Q_CHECK_PTR(m_page);
    setMainWidget(m_page);
    resize(m_page->sizeHint());

    m_page->lblColormodel->setText(layerCS);
    connect(m_page->radioCurrentLayer, SIGNAL(toggled(bool)), this, SLOT(slotSetColorSpaceLabel()));
    connect(m_page->radioAllLayers, SIGNAL(toggled(bool)), this, SLOT(slotSetColorSpaceLabel()));
    connect(m_page->chkColors, SIGNAL(toggled(bool)), this, SLOT(separateToColorActivated(bool)));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(okClicked()));

    KisDialogStateSaver::restoreState(m_page, "krita/separate channels");
}

DlgSeparate::~DlgSeparate()
{
    KisDialogStateSaver::saveState(m_page, "krita/separate channels");
    delete m_page;
}
enumSepAlphaOptions DlgSeparate::getAlphaOptions()
{
    if (m_page->radioCopyAlpha->isChecked()) return COPY_ALPHA_TO_SEPARATIONS;
    if (m_page->radioDiscardAlpha->isChecked()) return DISCARD_ALPHA;
    if (m_page->radioSeparateAlpha->isChecked()) return CREATE_ALPHA_SEPARATION;

    return COPY_ALPHA_TO_SEPARATIONS;
}

enumSepSource DlgSeparate::getSource()
{
    if (m_page->radioCurrentLayer->isChecked()) return CURRENT_LAYER;
    if (m_page->radioAllLayers->isChecked()) return ALL_LAYERS;
    return CURRENT_LAYER;
}

bool DlgSeparate::getDownscale()
{
    return m_page->chkDownscale->isChecked();
}

bool DlgSeparate::getToColor()
{
    return m_page->chkColors->isChecked();
}

bool DlgSeparate::getActivateCurrentChannel()
{
    return m_page->chkActivateCurrentChannel->isChecked();
}

void DlgSeparate::okClicked()
{
    accept();
}

void DlgSeparate::separateToColorActivated(bool disable)
{
    if (m_canDownScale) {
        m_page->chkDownscale->setDisabled(disable);
    }
    m_page->chkActivateCurrentChannel->setDisabled(!disable);
}

void DlgSeparate::slotSetColorSpaceLabel()
{
    if (m_page->radioCopyAlpha->isChecked()) {
        m_page->lblColormodel->setText(m_layerCS);
    } else if (m_page->radioAllLayers->isChecked()) {
        m_page->lblColormodel->setText(m_imageCS);
    }
}
void DlgSeparate::enableDownscale(bool enable)
{
    m_canDownScale = enable;
    m_page->chkDownscale->setEnabled(enable);
}

