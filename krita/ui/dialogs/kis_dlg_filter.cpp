/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

// From krita/image
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_filter_mask.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_pixel_selection.h>
#include <kis_paint_device.h>

#include "ui_wdgfilterdialog.h"

struct KisFilterDialog::Private {
    Private() 
        : currentFilter(0)
        , mask(0)
    {
    }

    KisFilterSP currentFilter;
    Ui_FilterDialog uiFilterDialog;
    KisFilterMaskSP mask;
    KisLayerSP layer;
};

KisFilterDialog::KisFilterDialog(QWidget* parent, KisLayerSP layer ) :
    QDialog( parent ),
    d( new Private )
{
    QRect rc = layer->extent();
    setModal( false );
    d->uiFilterDialog.setupUi( this );
    d->layer = layer;

    d->mask = new KisFilterMask();
    KisPixelSelectionSP psel = d->mask->selection()->getOrCreatePixelSelection();
    psel->select( rc );
    d->mask->selection()->updateProjection();
    
    d->layer->setPreviewMask( d->mask );
    d->uiFilterDialog.filterSelection->setLayer( d->layer );
    
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
    dbgKrita << "setFilter: " << f;
    d->currentFilter = f;
    d->uiFilterDialog.filterSelection->setFilter(f);
    updatePreview();
}

void KisFilterDialog::updatePreview()
{
    dbgKrita <<">>>>  KisFilterDialog::updatePreview()";

    if ( !d->currentFilter ) return;

    d->mask->setFilter( d->uiFilterDialog.filterSelection->configuration() );
    d->mask->setDirty(d->layer->extent());

}

void KisFilterDialog::apply()
{
    if ( !d->currentFilter ) return;
    
    KisFilterConfiguration* config = d->uiFilterDialog.filterSelection->configuration();
    emit(sigPleaseApplyFilter(d->layer, config));
}

void KisFilterDialog::close()
{
    d->layer->removePreviewMask();
    d->layer->setDirty(d->layer->extent());
}

#include "kis_dlg_filter.moc"
