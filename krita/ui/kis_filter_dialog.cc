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

#include "kis_filter_dialog.h"

#include <QTreeView>

// From krita/image
#include <kis_filter.h>
#include <kis_filter_config_widget.h>
#include <kis_filter_mask.h>
#include <kis_layer.h>

// From krita/ui
#include "kis_bookmarked_configurations_editor.h"
#include "kis_bookmarked_filter_configurations_model.h"
#include "kis_filters_model.h"

#include "ui_wdgfilterdialog.h"

struct KisFilterDialog::Private {
    Private() : currentCentralWidget(0), currentFilterConfigurationWidget(0),
            currentFilter(0), layer(0), mask(0), currentBookmarkedFilterConfigurationsModel(0), filtersModel(new KisFiltersModel)
    {
    }
    ~Private()
    {
        delete currentCentralWidget;
        delete widgetLayout;
        delete filtersModel;
    }
    QWidget* currentCentralWidget;
    KisFilterConfigWidget* currentFilterConfigurationWidget;
    KisFilterSP currentFilter;
    KisLayerSP layer;
    KisPaintDeviceSP thumb;
    Ui_FilterDialog uiFilterDialog;
    KisFilterMaskSP mask;
    QGridLayout *widgetLayout;
    KisBookmarkedFilterConfigurationsModel* currentBookmarkedFilterConfigurationsModel;
    KisFiltersModel* filtersModel;
};

KisFilterDialog::KisFilterDialog(QWidget* parent, KisLayerSP layer ) :
    QDialog( parent ),
    d( new Private )
{
    setModal( false );
    d->uiFilterDialog.setupUi( this );
    d->widgetLayout = new QGridLayout( d->uiFilterDialog.centralWidgetHolder );
    d->layer = layer;
    d->thumb = layer->paintDevice()->createThumbnailDevice(100, 100);
    d->mask = new KisFilterMask();
    d->layer->setPreviewMask( d->mask );
    d->uiFilterDialog.filtersSelector->setModel(d->filtersModel);
    connect(d->uiFilterDialog.comboBoxPresets, SIGNAL(activated ( int )), SLOT(slotBookmarkedFilterConfigurationSelected(int )) );
    connect(d->uiFilterDialog.pushButtonOk, SIGNAL(pressed()), SLOT(accept()));
    connect(d->uiFilterDialog.pushButtonOk, SIGNAL(pressed()), SLOT(apply()));
    connect(d->uiFilterDialog.pushButtonApply, SIGNAL(pressed()), SLOT(apply()));
    connect(d->uiFilterDialog.pushButtonCancel, SIGNAL(pressed()), SLOT(reject()));
    connect(d->uiFilterDialog.pushButtonEditPressets, SIGNAL(pressed()), SLOT(editConfigurations()));
}

KisFilterDialog::~KisFilterDialog()
{
    delete d;
}

void KisFilterDialog::setFilter(KisFilterSP f)
{
    d->currentFilter = f;
    delete d->currentCentralWidget;
    KisFilterConfigWidget* widget = d->currentFilter->createConfigurationWidget( d->uiFilterDialog.centralWidgetHolder, d->layer->paintDevice() );
    if(not widget)
    {
        d->currentFilterConfigurationWidget = 0;
        d->currentCentralWidget = new QLabel( i18n("No configuration option."), d->uiFilterDialog.centralWidgetHolder );
    } else {
        d->currentFilterConfigurationWidget = widget;
        d->currentCentralWidget = widget;
        d->currentFilterConfigurationWidget->setConfiguration( d->currentFilter->defaultConfiguration( d->layer->paintDevice() ) );
        connect(d->currentFilterConfigurationWidget, SIGNAL(sigPleaseUpdatePreview()), SLOT(updatePreview()));
    }
    delete d->currentBookmarkedFilterConfigurationsModel;
    d->currentBookmarkedFilterConfigurationsModel = new KisBookmarkedFilterConfigurationsModel(d->thumb, f );
    d->uiFilterDialog.comboBoxPresets->setModel(  d->currentBookmarkedFilterConfigurationsModel );
    d->widgetLayout->addWidget( d->currentCentralWidget, 0 , 0);
    d->uiFilterDialog.centralWidgetHolder->setMinimumSize( d->currentCentralWidget->minimumSize() );
    updatePreview();
}

void KisFilterDialog::updatePreview()
{
    kDebug(41007) <<"KisFilterDialog::updatePreview()";
    if( not d->currentFilter ) return;
    if( d->currentFilterConfigurationWidget )
    {
        KisFilterConfiguration* config = d->currentFilterConfigurationWidget->configuration();
        d->mask->setFilter( config );
    } else {
        d->mask->setFilter( d->currentFilter->defaultConfiguration( d->layer->paintDevice() ) );
    }
    d->mask->setDirty();
}

void KisFilterDialog::apply()
{
    if( not d->currentFilter ) return;
    KisFilterConfiguration* config = 0;
    if( d->currentFilterConfigurationWidget )
    {
        config = d->currentFilterConfigurationWidget->configuration();
    }
    emit(sigPleaseApplyFilter(d->layer, config));
}

void KisFilterDialog::slotBookmarkedFilterConfigurationSelected(int index)
{
    if(d->currentFilterConfigurationWidget)
    {
        QModelIndex modelIndex = d->currentBookmarkedFilterConfigurationsModel->index(index,0);
        KisFilterConfiguration* config  = d->currentBookmarkedFilterConfigurationsModel->configuration( modelIndex );
        d->currentFilterConfigurationWidget->setConfiguration( config );
    }
}

void KisFilterDialog::editConfigurations()
{
    KisSerializableConfiguration* config =
            d->currentFilterConfigurationWidget ? d->currentFilterConfigurationWidget->configuration() : 0;
    KisBookmarkedConfigurationsEditor editor(this, d->currentBookmarkedFilterConfigurationsModel, config);
    editor.exec();
}

#include "kis_filter_dialog.moc"
