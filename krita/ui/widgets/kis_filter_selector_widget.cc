/*
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_filter_selector_widget.h"

#include <QHeaderView>
#include <QTreeView>

#include "ui_wdgfilterselector.h"

#include <kis_layer.h>
#include <kis_paint_device.h>
#include <filter/kis_filter.h>
#include <kis_config_widget.h>
#include <filter/kis_filter_configuration.h>
#include <kis_image.h>

// From krita/ui
#include "kis_bookmarked_configurations_editor.h"
#include "kis_bookmarked_filter_configurations_model.h"
#include "kis_filters_model.h"
#include "kis_config.h"

struct KisFilterSelectorWidget::Private {
    QWidget* currentCentralWidget;
    KisConfigWidget* currentFilterConfigurationWidget;
    KisFilterSP currentFilter;
    KisImageWSP image;
    KisPaintDeviceSP paintDevice;
    Ui_FilterSelector uiFilterSelector;
    KisPaintDeviceSP thumb;
    KisBookmarkedFilterConfigurationsModel* currentBookmarkedFilterConfigurationsModel;
    KisFiltersModel* filtersModel;
    QGridLayout *widgetLayout;
};

KisFilterSelectorWidget::KisFilterSelectorWidget(QWidget* parent) : d(new Private)
{
    Q_UNUSED(parent);
    setObjectName("KisFilterSelectorWidget");
    d->currentCentralWidget = 0;
    d->currentFilterConfigurationWidget = 0;
    d->currentBookmarkedFilterConfigurationsModel = 0;
    d->currentFilter = 0;
    d->filtersModel = 0;
    d->uiFilterSelector.setupUi(this);

    d->widgetLayout = new QGridLayout(d->uiFilterSelector.centralWidgetHolder);

    connect(d->uiFilterSelector.filtersSelector, SIGNAL(entered(const QModelIndex&)), SLOT(setFilterIndex(const QModelIndex &)));
    connect(d->uiFilterSelector.filtersSelector, SIGNAL(clicked(const QModelIndex&)), SLOT(setFilterIndex(const QModelIndex &)));
    connect(d->uiFilterSelector.filtersSelector, SIGNAL(activated(const QModelIndex&)), SLOT(setFilterIndex(const QModelIndex &)));

    connect(d->uiFilterSelector.comboBoxPresets, SIGNAL(activated(int)),
            SLOT(slotBookmarkedFilterConfigurationSelected(int)));
    connect(d->uiFilterSelector.pushButtonEditPressets, SIGNAL(pressed()), SLOT(editConfigurations()));

    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    d->widgetLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum), 1, 0, 0, 2);
    d->widgetLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding), 0, 1, 2, 1);
}

KisFilterSelectorWidget::~KisFilterSelectorWidget()
{
    KisConfig cfg;
    QList<int> sizes = d->uiFilterSelector.splitter->sizes();
    cfg.setShowFilterGallery(sizes[0] > 0);
    delete d->filtersModel;
    delete d->currentCentralWidget;
    delete d->widgetLayout;
    delete d;
}

void KisFilterSelectorWidget::setPaintDevice(KisPaintDeviceSP _paintDevice)
{
    if (!_paintDevice) return;

    d->paintDevice = _paintDevice;
    d->thumb = d->paintDevice->createThumbnailDevice(100, 100);
    d->filtersModel = new KisFiltersModel(d->thumb);
    d->uiFilterSelector.filtersSelector->setModel(d->filtersModel);
    d->uiFilterSelector.filtersSelector->header()->setVisible(false);
}

void KisFilterSelectorWidget::setImage(KisImageWSP _image)
{
    d->image = _image;
}

void KisFilterSelectorWidget::showSelector(bool visible)
{
    QList<int> sizes;
    if (visible) {
        sizes << 200;
    } else {
        sizes << 0;
    }
    d->uiFilterSelector.splitter->setSizes(sizes);
}

void KisFilterSelectorWidget::setFilter(KisFilterSP f)
{
    Q_ASSERT(f);
    Q_ASSERT(d->filtersModel);
    setWindowTitle(f->name());
    dbgKrita << "setFilter: " << f;
    d->currentFilter = f;
    delete d->currentCentralWidget;

    {
        bool v = d->uiFilterSelector.filtersSelector->blockSignals(true);
        d->uiFilterSelector.filtersSelector->setCurrentIndex(d->filtersModel->indexForFilter(f->id()));
        d->uiFilterSelector.filtersSelector->blockSignals(v);
    }

    KisConfigWidget* widget =
        d->currentFilter->createConfigurationWidget(d->uiFilterSelector.centralWidgetHolder, d->paintDevice, d->image);

    if (!widget) { // No widget, so display a label instead
        d->currentFilterConfigurationWidget = 0;
        d->currentCentralWidget = new QLabel(i18n("No configuration option."),
                                             d->uiFilterSelector.centralWidgetHolder);
    } else {
        d->currentFilterConfigurationWidget = widget;
        d->currentCentralWidget = widget;
        d->currentFilterConfigurationWidget->setConfiguration(
            d->currentFilter->defaultConfiguration(d->paintDevice));
        connect(d->currentFilterConfigurationWidget, SIGNAL(sigConfigurationUpdated()), this, SIGNAL(configurationChanged()));
    }

    // Change the list of presets
    delete d->currentBookmarkedFilterConfigurationsModel;
    d->currentBookmarkedFilterConfigurationsModel = new KisBookmarkedFilterConfigurationsModel(d->thumb, f);
    d->uiFilterSelector.comboBoxPresets->setModel(d->currentBookmarkedFilterConfigurationsModel);

    // Add the widget to the layout
    d->currentCentralWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    d->widgetLayout->addWidget(d->currentCentralWidget, 0 , 0);

    KisConfig cfg;
    QList<int> sizes;
    if (cfg.showFilterGallery()) {

        sizes << d->uiFilterSelector.filtersSelector->sizeHint().width()
        << d->currentCentralWidget->sizeHint().width();
    } else {
        sizes << 0 << d->currentCentralWidget->sizeHint().width();
    }
    d->uiFilterSelector.splitter->setSizes(sizes);

}

void KisFilterSelectorWidget::setFilterIndex(const QModelIndex& idx)
{
    Q_ASSERT(d->filtersModel);
    KisFilter* filter = const_cast<KisFilter*>(d->filtersModel->indexToFilter(idx));
    if (filter) {
        setFilter(filter);
    } else {
        if (d->currentFilter) {
            bool v = d->uiFilterSelector.filtersSelector->blockSignals(true);
            QModelIndex idx = d->filtersModel->indexForFilter(d->currentFilter->id());
            d->uiFilterSelector.filtersSelector->setCurrentIndex(idx);
            d->uiFilterSelector.filtersSelector->scrollTo(idx);
            d->uiFilterSelector.filtersSelector->blockSignals(v);
        }
    }
    emit(configurationChanged());
}

void KisFilterSelectorWidget::slotBookmarkedFilterConfigurationSelected(int index)
{
    if (d->currentFilterConfigurationWidget) {
        QModelIndex modelIndex = d->currentBookmarkedFilterConfigurationsModel->index(index, 0);
        KisFilterConfiguration* config  = d->currentBookmarkedFilterConfigurationsModel->configuration(modelIndex);
        d->currentFilterConfigurationWidget->setConfiguration(config);
    }
}

void KisFilterSelectorWidget::editConfigurations()
{
    KisSerializableConfiguration* config =
        d->currentFilterConfigurationWidget ? d->currentFilterConfigurationWidget->configuration() : 0;
    KisBookmarkedConfigurationsEditor editor(this, d->currentBookmarkedFilterConfigurationsModel, config);
    editor.exec();
}

KisFilterConfiguration* KisFilterSelectorWidget::configuration()
{
    if (d->currentFilterConfigurationWidget) {
        KisFilterConfiguration * config
        = dynamic_cast<KisFilterConfiguration*>(d->currentFilterConfigurationWidget->configuration());
        if (config) {
            return config;
        }
    } else if (d->currentFilter) {
        return d->currentFilter->defaultConfiguration(d->paintDevice);
    }
    return 0;

}

#include "kis_filter_selector_widget.moc"
