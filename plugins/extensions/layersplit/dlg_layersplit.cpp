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

    KisConfig cfg(true);
    m_page->intFuzziness->setValue(cfg.readEntry<int>("layersplit/fuzziness", 20));
    m_page->chkCreateGroupLayer->setChecked(cfg.readEntry<bool>("layerspit/createmastergroup", true));
    m_page->chkSeparateGroupLayers->setChecked(cfg.readEntry<bool>("layerspit/separategrouplayers", false));
    m_page->chkAlphaLock->setChecked(cfg.readEntry<bool>("layerspit/alphalock", true));
    m_page->chkHideOriginal->setChecked(cfg.readEntry<bool>("layerspit/hideoriginal", false));
    m_page->chkSortLayers->setChecked(cfg.readEntry<bool>("layerspit/sortlayers", true));
    m_page->chkDisregardOpacity->setChecked(cfg.readEntry<bool>("layerspit/disregardopacity", true));

    QString paletteName = cfg.readEntry<QString>("layersplit/paletteName", i18n("Default"));
    KoResourceServer<KoColorSet> *pserver = KoResourceServerProvider::instance()->paletteServer();
    KoColorSet *pal = pserver->resourceByName(paletteName);
    if (pal) {
        m_palette = pal;
        m_page->paletteChooser->setText(pal->name());
        QIcon icon(QPixmap::fromImage(pal->image()));
        m_page->paletteChooser->setIcon(icon);
    }


    connect(this, SIGNAL(applyClicked()), this, SLOT(applyClicked()));

    setMainWidget(m_page);
}

DlgLayerSplit::~DlgLayerSplit()
{
}

void DlgLayerSplit::applyClicked()
{
    KisConfig cfg(false);
    cfg.writeEntry("layersplit/fuzziness", m_page->intFuzziness->value());
    cfg.writeEntry("layerspit/createmastergroup", m_page->chkCreateGroupLayer->isChecked());
    cfg.writeEntry("layerspit/separategrouplayers", m_page->chkSeparateGroupLayers->isChecked());
    cfg.writeEntry("layerspit/alphalock", m_page->chkAlphaLock->isChecked());
    cfg.writeEntry("layerspit/hideoriginal", m_page->chkHideOriginal->isChecked());
    cfg.writeEntry("layerspit/sortlayers", m_page->chkSortLayers->isChecked());
    cfg.writeEntry("layerspit/disregardopacity", m_page->chkDisregardOpacity->isChecked());
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
