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



#include <KoResourcePaths.h>
#include <QPushButton>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_filter_mask.h>
#include <kis_node.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <KisViewManager.h>
#include <kis_config.h>

#include "kis_selection.h"
#include "kis_node_commands_adapter.h"
#include "kis_filter_manager.h"
#include "ui_wdgfilterdialog.h"
#include "kis_canvas2.h"


struct KisDlgFilter::Private {
    Private(KisFilterManager *_filterManager, KisViewManager *_view)
            : currentFilter(0)
            , resizeCount(0)
            , view(_view)
            , filterManager(_filterManager)
            , blockModifyingActionsGuard(new KisInputActionGroupsMaskGuard(view->canvasBase(), ViewTransformActionGroup))
    {
    }

    KisFilterSP currentFilter;
    Ui_FilterDialog uiFilterDialog;
    KisNodeSP node;
    int resizeCount;
    KisViewManager *view;
    KisFilterManager *filterManager;

    // a special guard object that blocks all the painting input actions while the
    // dialog is open
    QScopedPointer<KisInputActionGroupsMaskGuard> blockModifyingActionsGuard;
};

KisDlgFilter::KisDlgFilter(KisViewManager *view, KisNodeSP node, KisFilterManager *filterManager, QWidget *parent) :
        QDialog(parent),
        d(new Private(filterManager, view))
{
    setModal(false);

    d->uiFilterDialog.setupUi(this);
    d->node = node;

    d->uiFilterDialog.filterSelection->setView(view);
    d->uiFilterDialog.filterSelection->showFilterGallery(KisConfig(true).showFilterGallery());

    d->uiFilterDialog.pushButtonCreateMaskEffect->show();
    connect(d->uiFilterDialog.pushButtonCreateMaskEffect, SIGNAL(pressed()), SLOT(createMask()));
    connect(d->uiFilterDialog.pushButtonCreateMaskEffect,SIGNAL(pressed()),SLOT(close()));

    d->uiFilterDialog.filterGalleryToggle->setChecked(d->uiFilterDialog.filterSelection->isFilterGalleryVisible());
    d->uiFilterDialog.filterGalleryToggle->setIcon(QPixmap(":/pics/sidebaricon.png"));
    d->uiFilterDialog.filterGalleryToggle->setMaximumWidth(d->uiFilterDialog.filterGalleryToggle->height());
    connect(d->uiFilterDialog.filterSelection, SIGNAL(sigFilterGalleryToggled(bool)), d->uiFilterDialog.filterGalleryToggle, SLOT(setChecked(bool)));
    connect(d->uiFilterDialog.filterGalleryToggle, SIGNAL(toggled(bool)), d->uiFilterDialog.filterSelection, SLOT(showFilterGallery(bool)));
    connect(d->uiFilterDialog.filterSelection, SIGNAL(sigSizeChanged()), this, SLOT(slotFilterWidgetSizeChanged()));

    if (node->inherits("KisMask")) {
        d->uiFilterDialog.pushButtonCreateMaskEffect->setVisible(false);
    }

    d->uiFilterDialog.filterSelection->setPaintDevice(true, d->node->original());

    connect(d->uiFilterDialog.buttonBox, SIGNAL(accepted()), SLOT(accept()));
    connect(d->uiFilterDialog.buttonBox, SIGNAL(rejected()), SLOT(reject()));
    connect(d->uiFilterDialog.checkBoxPreview, SIGNAL(toggled(bool)), SLOT(enablePreviewToggled(bool)));

    connect(d->uiFilterDialog.filterSelection, SIGNAL(configurationChanged()), SLOT(filterSelectionChanged()));

    connect(this, SIGNAL(accepted()), SLOT(slotOnAccept()));
    connect(this, SIGNAL(rejected()), SLOT(slotOnReject()));

    KConfigGroup group( KSharedConfig::openConfig(), "filterdialog");
    d->uiFilterDialog.checkBoxPreview->setChecked(group.readEntry("showPreview", true));

    restoreGeometry(KisConfig(true).readEntry("filterdialog/geometry", QByteArray()));

}

KisDlgFilter::~KisDlgFilter()
{
    KisConfig(false).writeEntry("filterdialog/geometry", saveGeometry());
    delete d;
}

void KisDlgFilter::setFilter(KisFilterSP f)
{
    Q_ASSERT(f);
    setDialogTitle(f);
    d->uiFilterDialog.filterSelection->setFilter(f);
    d->uiFilterDialog.pushButtonCreateMaskEffect->setEnabled(f->supportsAdjustmentLayers());
    d->currentFilter = f;
    updatePreview();
}

void KisDlgFilter::setDialogTitle(KisFilterSP filter)
{
    setWindowTitle(filter.isNull() ? i18nc("@title:window", "Filter") : i18nc("@title:window", "Filter: %1", filter->name()));
}

void KisDlgFilter::startApplyingFilter(KisFilterConfigurationSP config)
{
    if (!d->uiFilterDialog.filterSelection->configuration()) return;

    if (d->node->inherits("KisPaintLayer")) {
        config->setChannelFlags(qobject_cast<KisPaintLayer*>(d->node.data())->channelLockFlags());
    }

    d->filterManager->apply(config);
}

void KisDlgFilter::updatePreview()
{
    KisFilterConfigurationSP config = d->uiFilterDialog.filterSelection->configuration();
    if (!config) return;

    bool maskCreationAllowed = !d->currentFilter || d->currentFilter->configurationAllowedForMask(config);
    d->uiFilterDialog.pushButtonCreateMaskEffect->setEnabled(maskCreationAllowed);

    if (d->uiFilterDialog.checkBoxPreview->isChecked()) {
        KisFilterConfigurationSP config(d->uiFilterDialog.filterSelection->configuration());
        startApplyingFilter(config);
    }

    d->uiFilterDialog.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

void KisDlgFilter::adjustSize()
{
    QWidget::adjustSize();
}

void KisDlgFilter::slotFilterWidgetSizeChanged()
{
    QMetaObject::invokeMethod(this, "adjustSize", Qt::QueuedConnection);
}

void KisDlgFilter::slotOnAccept()
{
    if (!d->filterManager->isStrokeRunning()) {
        KisFilterConfigurationSP config(d->uiFilterDialog.filterSelection->configuration());
        startApplyingFilter(config);
    }

    d->filterManager->finish();

    d->uiFilterDialog.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    KisConfig(false).setShowFilterGallery(d->uiFilterDialog.filterSelection->isFilterGalleryVisible());
}

void KisDlgFilter::slotOnReject()
{
    if (d->filterManager->isStrokeRunning()) {
        d->filterManager->cancel();
    }

    KisConfig(false).setShowFilterGallery(d->uiFilterDialog.filterSelection->isFilterGalleryVisible());
}

void KisDlgFilter::createMask()
{
    if (d->node->inherits("KisMask")) return;

    if (d->filterManager->isStrokeRunning()) {
        d->filterManager->cancel();
    }

    KisLayer *layer = qobject_cast<KisLayer*>(d->node.data());
    KisFilterMaskSP mask = new KisFilterMask();
    mask->setName(d->currentFilter->name());
    mask->initSelection(d->view->selection(), layer);
    mask->setFilter(d->uiFilterDialog.filterSelection->configuration());

    Q_ASSERT(layer->allowAsChild(mask));

    KisNodeCommandsAdapter adapter(d->view);
    adapter.addNode(mask, layer, layer->lastChild());
}

void KisDlgFilter::enablePreviewToggled(bool state)
{
    if (state) {
        updatePreview();
    } else if (d->filterManager->isStrokeRunning()) {
        d->filterManager->cancel();
    }

    KConfigGroup group( KSharedConfig::openConfig(), "filterdialog");
    group.writeEntry("showPreview", d->uiFilterDialog.checkBoxPreview->isChecked());

    group.config()->sync();
}

void KisDlgFilter::filterSelectionChanged()
{
    KisFilterSP filter = d->uiFilterDialog.filterSelection->currentFilter();
    setDialogTitle(filter);
    d->uiFilterDialog.pushButtonCreateMaskEffect->setEnabled(filter.isNull() ? false : filter->supportsAdjustmentLayers());
    updatePreview();
}


void KisDlgFilter::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);

//    // Workaround, after the initialisation don't center the dialog anymore
//    if(d->resizeCount < 2) {
//        QWidget* canvas = d->view->canvas();
//        QRect rect(canvas->mapToGlobal(canvas->geometry().topLeft()), size());
//        int deltaX = (canvas->geometry().width() - geometry().width())/2;
//        int deltaY = (canvas->geometry().height() - geometry().height())/2;
//        rect.translate(deltaX, deltaY);
//        setGeometry(rect);

//        d->resizeCount++;
//    }
}
