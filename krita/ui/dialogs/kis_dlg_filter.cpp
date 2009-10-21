/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdysa.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_dlg_filter.h"

#include <KoCompositeOp.h>

// From krita/image
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_filter_mask.h>
#include <kis_node.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_pixel_selection.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include "commands/kis_image_layer_add_command.h"
#include "kis_undo_adapter.h"
#include "ui_wdgfilterdialog.h"

struct KisFilterDialog::Private {
    Private()
            : currentFilter(0)
            , mask(0) {
    }

    KisFilterSP currentFilter;
    Ui_FilterDialog uiFilterDialog;
    KisFilterMaskSP mask;
    KisNodeSP node;
    KisImageWSP image;
};

KisFilterDialog::KisFilterDialog(QWidget* parent, KisNodeSP node, KisImageWSP image, KisSelectionSP selection) :
        QDialog(parent),
        d(new Private)
{
    QRect rc = node->extent();
    setModal(false);

    d->uiFilterDialog.setupUi(this);
    d->node = node;
    d->image = image;
    d->mask = new KisFilterMask();

    KisPixelSelectionSP psel = d->mask->selection()->getOrCreatePixelSelection();
    if (selection) {
        QRect extent = selection->selectedRect();
        KisPainter painter(psel);
        painter.setCompositeOp(selection->colorSpace()->compositeOp(COMPOSITE_COPY));
        painter.bitBlt(extent.topLeft(), selection, extent);
        painter.end();
    } else {
        psel->select(rc);
    }
    d->mask->selection()->updateProjection();

    if (d->node->inherits("KisLayer")) {
        qobject_cast<KisLayer*>(d->node.data())->setPreviewMask(d->mask);
        d->uiFilterDialog.pushButtonCreateMaskEffect->show();
        d->uiFilterDialog.pushButtonCreateMaskEffect->setEnabled(true);
        connect(d->uiFilterDialog.pushButtonCreateMaskEffect, SIGNAL(pressed()), SLOT(createMask()));
    } else {
        d->uiFilterDialog.pushButtonCreateMaskEffect->hide();
    }
    d->uiFilterDialog.pushButtonCreateMaskEffect->hide(); // TODO fixme, understand why the mask isn't created, and then remove that line
    d->uiFilterDialog.filterSelection->setPaintDevice(d->node->paintDevice());
    d->uiFilterDialog.filterSelection->setImage(d->image);

    connect(d->uiFilterDialog.pushButtonOk, SIGNAL(pressed()), SLOT(accept()));
    connect(d->uiFilterDialog.pushButtonOk, SIGNAL(pressed()), SLOT(close()));
    connect(d->uiFilterDialog.pushButtonOk, SIGNAL(pressed()), SLOT(apply()));
    connect(d->uiFilterDialog.pushButtonApply, SIGNAL(pressed()), SLOT(apply()));
    connect(d->uiFilterDialog.pushButtonCancel, SIGNAL(pressed()), SLOT(close()));
    connect(d->uiFilterDialog.pushButtonCancel, SIGNAL(pressed()), SLOT(reject()));

    connect(d->uiFilterDialog.filterSelection, SIGNAL(configurationChanged()), SLOT(updatePreview()));
}

KisFilterDialog::~KisFilterDialog()
{
    delete d;
}

void KisFilterDialog::setFilter(KisFilterSP f)
{
    Q_ASSERT(f);
    setWindowTitle(f->name());
    d->currentFilter = f;
    d->uiFilterDialog.filterSelection->setFilter(f);
    updatePreview();
}

void KisFilterDialog::updatePreview()
{
    if (!d->currentFilter) return;

    d->mask->setFilter(d->uiFilterDialog.filterSelection->configuration());
    d->mask->setDirty();
    d->uiFilterDialog.pushButtonOk->setEnabled(true);
    d->uiFilterDialog.pushButtonApply->setText(i18n("Apply"));
}

void KisFilterDialog::apply()
{
    if (!d->currentFilter) return;

    KisFilterConfiguration* config = d->uiFilterDialog.filterSelection->configuration();
    emit(sigPleaseApplyFilter(d->node, config));
    d->uiFilterDialog.pushButtonOk->setEnabled(false);
    d->uiFilterDialog.pushButtonApply->setText(i18n("Apply Again"));
}

void KisFilterDialog::close()
{
    if (d->node->inherits("KisLayer")) {
        qobject_cast<KisLayer*>(d->node.data())->removePreviewMask();
    }
    d->node->setDirty(d->node->extent());
}

void KisFilterDialog::createMask()
{
    KisEffectMaskSP mask;
    if (d->node->inherits("KisLayer")) {
        KisLayer * l = qobject_cast<KisLayer*>(d->node.data());
        mask = l->previewMask();
        l->removePreviewMask();
        d->image->undoAdapter()->addCommand(new KisImageLayerAddCommand(d->image, mask, l, KisNodeSP(0)));
        mask->setDirty();
        close();
        accept();
    }
}


#include "kis_dlg_filter.moc"
