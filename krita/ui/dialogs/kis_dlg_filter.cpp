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

#include <kguiitem.h>
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
#include <kis_config.h>
#include <kis_canvas2.h>
#include <kis_view2.h>

struct KisFilterDialog::Private {
    Private()
            : currentFilter(0)
            , mask(0)
            , resizeCount(0)
            , view(0) {
    }

    KisFilterSP currentFilter;
    Ui_FilterDialog uiFilterDialog;
    KisFilterMaskSP mask;
    KisNodeSP node;
    KisImageWSP image;
    int resizeCount;
    KisView2 *view;
};

KisFilterDialog::KisFilterDialog(KisView2 *view, KisNodeSP node, KisImageWSP image, KisSelectionSP selection) :
        QDialog(view),
        d(new Private)
{
    setModal(false);

    d->uiFilterDialog.setupUi(this);
    d->node = node;
    d->image = image;
    d->view = view;
    d->mask = new KisFilterMask();
    d->mask->initSelection(selection, dynamic_cast<KisLayer*>(node.data()));
    d->uiFilterDialog.filterSelection->setView(view);
    d->uiFilterDialog.filterSelection->showFilterGallery(KisConfig().showFilterGallery());

    if (d->node->inherits("KisLayer")) {
        qobject_cast<KisLayer*>(d->node.data())->setPreviewMask(d->mask);
        d->uiFilterDialog.pushButtonCreateMaskEffect->show();
        d->uiFilterDialog.pushButtonCreateMaskEffect->setEnabled(true);
        connect(d->uiFilterDialog.pushButtonCreateMaskEffect, SIGNAL(pressed()), SLOT(createMask()));
    } else {
        d->uiFilterDialog.pushButtonCreateMaskEffect->hide();
    }
    d->uiFilterDialog.pushButtonCreateMaskEffect->hide(); // TODO fixme, understand why the mask isn't created, and then remove that line
    d->uiFilterDialog.filterSelection->setPaintDevice(d->node->original());
    d->uiFilterDialog.pushButtonOk->setGuiItem(KStandardGuiItem::ok());
    d->uiFilterDialog.pushButtonCancel->setGuiItem(KStandardGuiItem::cancel());

    connect(d->uiFilterDialog.pushButtonOk, SIGNAL(pressed()), SLOT(apply()));
    connect(d->uiFilterDialog.pushButtonOk, SIGNAL(pressed()), SLOT(accept()));
    connect(d->uiFilterDialog.pushButtonCancel, SIGNAL(pressed()), SLOT(reject()));
    connect(d->uiFilterDialog.checkBoxPreview, SIGNAL(stateChanged(int)), SLOT(previewCheckBoxChange(int)));

    connect(d->uiFilterDialog.filterSelection, SIGNAL(configurationChanged()), SLOT(updatePreview()));
    connect(this, SIGNAL(finished(int)), SLOT(close()));

    KConfigGroup group(KGlobal::config(), "filterdialog");
    d->uiFilterDialog.checkBoxPreview->setChecked(group.readEntry("showPreview", true));
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
    
    if(d->uiFilterDialog.checkBoxPreview->isChecked()) {
        d->mask->setFilter(d->uiFilterDialog.filterSelection->configuration());
        d->mask->setDirty();
        d->node->setDirty(d->node->extent());
    }
    
    d->uiFilterDialog.pushButtonOk->setEnabled(true);
}

void KisFilterDialog::apply()
{
    if (!d->currentFilter) return;

    KisSafeFilterConfigurationSP config(d->uiFilterDialog.filterSelection->configuration());
    if (d->node->inherits("KisLayer")) {
        config->setChannelFlags(qobject_cast<KisLayer*>(d->node.data())->channelFlags());
    }
    emit(sigPleaseApplyFilter(config));
    d->uiFilterDialog.pushButtonOk->setEnabled(false);
}

void KisFilterDialog::close()
{
    if (d->node->inherits("KisLayer")) {
        qobject_cast<KisLayer*>(d->node.data())->removePreviewMask();
    }
    d->node->setDirty(d->node->extent());
    KisConfig().setShowFilterGallery(d->uiFilterDialog.filterSelection->isFilterGalleryVisible());
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

void KisFilterDialog::previewCheckBoxChange(int state)
{
    d->mask->setVisible(state == Qt::Checked);
    updatePreview();
    if (state != Qt::Checked) {
        // update node to hide what remains from the filter mask
        d->node->setDirty(d->node->extent());
    }

    KConfigGroup group(KGlobal::config(), "filterdialog");
    group.writeEntry("showPreview", d->uiFilterDialog.checkBoxPreview->isChecked());
    group.config()->sync();
}

void KisFilterDialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);

    // Workaround, after the initalisation don't center the dialog anymore
    if(d->resizeCount < 2) {
        QWidget* canvas = d->view->canvas();
        QRect rect(canvas->mapToGlobal(canvas->geometry().topLeft()), size());
        int deltaX = (canvas->geometry().width() - geometry().width())/2;
        int deltaY = (canvas->geometry().height() - geometry().height())/2;
        rect.translate(deltaX, deltaY);
        setGeometry(rect);

        d->resizeCount++;
    }
}



#include "kis_dlg_filter.moc"
