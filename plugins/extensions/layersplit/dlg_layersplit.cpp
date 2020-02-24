/*
 * Copyright (C) 2014 Boudewijn Rempt <boud@valdyas.org>
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

    m_colorSetChooser = new KisPaletteListWidget();
    m_page->paletteChooser->setPopupWidget(m_colorSetChooser);

    connect(m_colorSetChooser, SIGNAL(sigPaletteSelected(KoColorSet*)), this, SLOT(slotSetPalette(KoColorSet*)));

    KisDialogStateSaver::restoreState(m_page, "krita/layer_split");

    connect(m_page->cmbMode, SIGNAL(currentIndexChanged(int)), this, SLOT(slotChangeMode(int)));

    KisConfig cfg(true);
    QString paletteName = cfg.readEntry<QString>("layersplit/paletteName", i18n("Default"));
    KoResourceServer<KoColorSet> *pserver = KoResourceServerProvider::instance()->paletteServer();
    KoColorSet *pal = pserver->resourceByName(paletteName);
    modeToMask = m_page->cmbMode->currentIndex();
    slotChangeMode(modeToMask);
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

KoColorSet *DlgLayerSplit::palette() const
{
    return m_palette;
}

void DlgLayerSplit::slotSetPalette(KoColorSet *pal)
{
    if (pal) {
        m_palette = pal;
        m_page->paletteChooser->setText(pal->name());
        QIcon icon(QPixmap::fromImage(pal->image()));
        m_page->paletteChooser->setIcon(icon);
    }
}

void DlgLayerSplit::slotChangeMode(int idx){
    modeToMask = idx;
    if( modeToMask){
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
