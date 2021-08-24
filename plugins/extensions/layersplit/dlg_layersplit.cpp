/*
 * SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "dlg_layersplit.h"

#include <klocalizedstring.h>

#include <KoResourceServerProvider.h>
#include <kis_debug.h>

#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_paint_device.h>

#include "kis_slider_spin_box.h"
#include <QCheckBox>
#include <QSpinBox>

#include <kis_config.h>
#include <KisDialogStateSaver.h>

DlgLayerSplit::DlgLayerSplit()
    : KoDialog()
{
    m_page = new WdgLayerSplit(this);

    setCaption(i18n("Split Layer"));
    setButtons(Apply | Cancel);
    setDefaultButton(Apply);

    m_page->intFuzziness->setRange(0, 200);
    m_page->intFuzziness->setSingleStep(1);

    m_colorSetChooser = new KisPaletteChooser();
    m_page->paletteChooser->setPopupWidget(m_colorSetChooser);
    connect(m_colorSetChooser, SIGNAL(sigPaletteSelected(KoColorSetSP)), this, SLOT(slotSetPalette(KoColorSetSP)));

    QMap<QString, QVariant> defaults;
    defaults[ m_page->intFuzziness->objectName() ] = QVariant::fromValue<int>(20);
    defaults[ m_page->chkCreateGroupLayer->objectName() ] = QVariant::fromValue<bool>(true);
    defaults[ m_page->chkSeparateGroupLayers->objectName() ] = QVariant::fromValue<bool>(false);
    defaults[ m_page->chkAlphaLock->objectName() ] = QVariant::fromValue<bool>(true);
    defaults[ m_page->chkHideOriginal->objectName() ] = QVariant::fromValue<bool>(false);
    defaults[ m_page->chkSortLayers->objectName() ] = QVariant::fromValue<bool>(true);
    defaults[ m_page->chkDisregardOpacity->objectName() ] = QVariant::fromValue<bool>(true);

    KisDialogStateSaver::restoreState(m_page, "krita/layer_split", defaults);

    connect(m_page->cmbMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChangeMode(int)));

    KisConfig cfg(true);
    QString paletteName = cfg.readEntry<QString>("layersplit/paletteName", "Default"); // resource names are not translatable by design
    KoResourceServer<KoColorSet> *pserver = KoResourceServerProvider::instance()->paletteServer();
    KoColorSetSP pal = pserver->resource("", "", paletteName);
    m_modeToMask = m_page->cmbMode->currentIndex();
    slotChangeMode(m_modeToMask);

    if (pal) {
        m_palette = pal;
        m_page->paletteChooser->setText(pal->name());
        QIcon icon(QPixmap::fromImage(pal->image()));
        m_page->paletteChooser->setIcon(icon);
    }


    connect(this, SIGNAL(applyClicked()), this, SLOT(slotApplyClicked()));

    setMainWidget(m_page);
}

DlgLayerSplit::~DlgLayerSplit()
{
}

void DlgLayerSplit::slotApplyClicked()
{
    
    KisDialogStateSaver::saveState(m_page, "krita/layer_split");

    KisConfig cfg(false);
    if (m_palette) {
        cfg.writeEntry("layersplit/paletteName", m_palette->name());
    }

    accept();
}

bool DlgLayerSplit::createBaseGroup() const
{
    return m_page->chkCreateGroupLayer->isChecked();
}

bool DlgLayerSplit::createSeparateGroups() const
{
    return m_page->chkSeparateGroupLayers->isChecked();
}

bool DlgLayerSplit::lockAlpha() const
{
    return m_page->chkAlphaLock->isChecked();
}

bool DlgLayerSplit::hideOriginal() const
{
    return m_page->chkHideOriginal->isChecked();
}

bool DlgLayerSplit::sortLayers() const
{
    return m_page->chkSortLayers->isChecked();
}

bool DlgLayerSplit::disregardOpacity() const
{
    return m_page->chkDisregardOpacity->isChecked();
}

int DlgLayerSplit::fuzziness() const
{
    return m_page->intFuzziness->value();

}

KoColorSetSP DlgLayerSplit::palette() const
{
    return m_palette;
}

void DlgLayerSplit::slotSetPalette(KoColorSetSP pal)
{
    if (pal) {
        m_palette = pal;
        m_page->paletteChooser->setText(pal->name());
        QIcon icon(QPixmap::fromImage(pal->image()));
        m_page->paletteChooser->setIcon(icon);
    }
}

void DlgLayerSplit::slotChangeMode(int idx){
    m_modeToMask = idx;
    if( m_modeToMask){
        m_page->chkCreateGroupLayer->hide();
        m_page->chkSeparateGroupLayers->hide();
        m_page->chkAlphaLock->hide();
        m_page->chkHideOriginal->hide();
    }
    else{
        m_page->chkCreateGroupLayer->show();
        m_page->chkSeparateGroupLayers->show();
        m_page->chkAlphaLock->show();
        m_page->chkHideOriginal->show();
    }
}
