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

#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_filter_mask.h>
#include <kis_node.h>
#include <kis_layer.h>
#include <kis_view2.h>
#include <kis_config.h>

#include "kis_selection.h"
#include "kis_node_commands_adapter.h"
#include "kis_filter_manager.h"
#include "ui_wdgfilterdialog.h"


struct KisFilterDialog::Private {
    Private()
            : currentFilter(0)
            , resizeCount(0)
            , view(0)
    {
    }

    KisFilterSP currentFilter;
    Ui_FilterDialog uiFilterDialog;
    KisNodeSP node;
    int resizeCount;
    KisView2 *view;
    KisFilterManager *filterManager;
};

KisFilterDialog::KisFilterDialog(KisView2 *view, KisNodeSP node, KisFilterManager *filterManager) :
        QDialog(view),
        d(new Private)
{
    setModal(false);

    d->uiFilterDialog.setupUi(this);
    d->node = node;
    d->view = view;
    d->filterManager = filterManager;

    d->uiFilterDialog.filterSelection->setView(view);
    d->uiFilterDialog.filterSelection->showFilterGallery(KisConfig().showFilterGallery());

    d->uiFilterDialog.pushButtonCreateMaskEffect->show();
    connect(d->uiFilterDialog.pushButtonCreateMaskEffect, SIGNAL(pressed()), SLOT(createMask()));

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
    d->uiFilterDialog.filterSelection->setFilter(f);
    updatePreview();
}

void KisFilterDialog::startApplyingFilter(KisSafeFilterConfigurationSP config)
{
    if (!d->uiFilterDialog.filterSelection->configuration()) return;

    if (d->node->inherits("KisLayer")) {
        config->setChannelFlags(qobject_cast<KisLayer*>(d->node.data())->channelFlags());
    }

    d->filterManager->apply(config);
}

void KisFilterDialog::updatePreview()
{
    if (!d->uiFilterDialog.filterSelection->configuration()) return;

    if (d->uiFilterDialog.checkBoxPreview->isChecked()) {
        KisSafeFilterConfigurationSP config(d->uiFilterDialog.filterSelection->configuration());
        startApplyingFilter(config);
    }

    d->uiFilterDialog.pushButtonOk->setEnabled(true);
}

void KisFilterDialog::apply()
{
    if (!d->filterManager->isStrokeRunning()) {
        KisSafeFilterConfigurationSP config(d->uiFilterDialog.filterSelection->configuration());
        startApplyingFilter(config);
    }
    d->filterManager->finish();

    d->uiFilterDialog.pushButtonOk->setEnabled(false);
}

void KisFilterDialog::close()
{
    if (d->filterManager->isStrokeRunning()) {
        d->filterManager->cancel();
    }

    KisConfig().setShowFilterGallery(d->uiFilterDialog.filterSelection->isFilterGalleryVisible());
}

void KisFilterDialog::createMask()
{
    if (d->filterManager->isStrokeRunning()) {
        d->filterManager->cancel();
    }

    KisLayer *layer = dynamic_cast<KisLayer*>(d->node.data());
    KisFilterMaskSP mask = new KisFilterMask();
    mask->initSelection(d->view->selection(), layer);
    mask->setFilter(d->uiFilterDialog.filterSelection->configuration());

    Q_ASSERT(layer->allowAsChild(mask));

    KisNodeCommandsAdapter adapter(d->view);
    adapter.addNode(mask, layer, layer->lastChild());
    accept();
}

void KisFilterDialog::previewCheckBoxChange(int state)
{
    if (state) {
        updatePreview();
    } else if (d->filterManager->isStrokeRunning()) {
        d->filterManager->cancel();
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
