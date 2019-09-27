/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "FiltersCategoryModel.h"
#include "FiltersModel.h"
#include <PropertyContainer.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_filter_mask.h>
#include <kis_layer.h>
#include <kis_selection.h>
#include <kis_config_widget.h>
#include <KisViewManager.h>
#include <kis_node_manager.h>
#include <kis_selection_manager.h>
#include <kis_canvas2.h>
#include <kis_filter_manager.h>

#include <QApplication>

#include <algorithm>

bool categoryLessThan(const FiltersModel* s1, const FiltersModel* s2)
{
    return s1->categoryName.toLower() < s2->categoryName.toLower();
}

class FiltersCategoryModel::Private
{
public:
    Private(FiltersCategoryModel* qq)
        : q(qq)
        , currentCategory(-1)
        , view(0)
        , previewEnabled(false)
        , previewFilterID(-1)
        , previewTimer(new QTimer())
    {
        previewTimer->setInterval(150);
        previewTimer->setSingleShot(true);
        connect(previewTimer, SIGNAL(timeout()), q, SLOT(updatePreview()));
    }

    FiltersCategoryModel* q;
    int currentCategory;
    KisViewManager* view;
    QList<FiltersModel*> categories;
    FiltersModel* categoryByName(const QString& name)
    {
        FiltersModel* category = 0;
        for(int i = 0; i < categories.count(); ++i)
        {
            if (categories.at(i)->categoryId == name)
            {
                category = categories[i];
                break;
            }
        }
        return category;
    }

    void refreshContents()
    {
        q->beginResetModel();
        qDeleteAll(categories);
        categories.clear();
        QList<KisFilterSP> filters = KisFilterRegistry::instance()->values();
        QList<QString> tmpCategoryIDs;
        Q_FOREACH (const KisFilterSP filter, filters) {
            Q_ASSERT(filter);
            FiltersModel* cat = 0;
            if (!tmpCategoryIDs.contains(filter->menuCategory().id())) {
                cat = new FiltersModel(q);
                cat->categoryId = filter->menuCategory().id();
                cat->categoryName = filter->menuCategory().name();
                cat->setView(view);
                categories << cat;
                tmpCategoryIDs << filter->menuCategory().id();
                connect(cat, SIGNAL(configurationChanged(int)), q, SLOT(filterConfigurationChanged(int)));
                connect(cat, SIGNAL(filterActivated(int)), q, SLOT(filterActivated(int)));
            }
            else
                cat = categoryByName(filter->menuCategory().id());
            cat->addFilter(filter);
            qApp->processEvents();
        }
        std::sort(categories.begin(), categories.end(), categoryLessThan);
        q->endResetModel();
    }

    bool previewEnabled;
    KisFilterMaskSP mask;
    KisNodeSP node;
    int previewFilterID;
    KisFilterConfigurationSP newConfig;
    QTimer* previewTimer;
};

FiltersCategoryModel::FiltersCategoryModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{
}

FiltersCategoryModel::~FiltersCategoryModel()
{
    delete d;
}

QHash<int, QByteArray> FiltersCategoryModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TextRole] = "text";

    return roles;
}

QVariant FiltersCategoryModel::data(const QModelIndex& index, int role) const
{
    QVariant data;
    if (index.isValid())
    {
        switch(role)
        {
            case TextRole:
                data = d->categories[index.row()]->categoryName;
                break;
            default:
                break;
        }
    }
    return data;
}

int FiltersCategoryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return d->categories.count();
}

QObject* FiltersCategoryModel::filterModel() const
{
    if (d->currentCategory == -1)
        return 0;
    return d->categories[d->currentCategory];
}

void FiltersCategoryModel::activateItem(int index)
{
    if (index > -1 && index < d->categories.count())
    {
        d->currentCategory = index;
        emit filterModelChanged();
    }
}

QObject* FiltersCategoryModel::view() const
{
    return d->view;
}

void FiltersCategoryModel::setView(QObject* newView)
{
    if (d->view)
    {
        setPreviewEnabled(false);
        d->view->nodeManager()->disconnect(this);
        d->view->selectionManager()->disconnect(this);
    }
    d->view = qobject_cast<KisViewManager*>( newView );
    if (d->view)
    {
        d->refreshContents();
        connect(d->view->nodeManager(), SIGNAL(sigLayerActivated(KisLayerSP)), this, SLOT(activeLayerChanged(KisLayerSP)));
        connect(d->view->selectionManager(), SIGNAL(currentSelectionChanged()), this, SLOT(activeSelectionChanged()));
    }
    emit viewChanged();
}

void FiltersCategoryModel::activeLayerChanged(KisLayerSP layer)
{
    Q_UNUSED(layer);
    setPreviewEnabled(false);
}

void FiltersCategoryModel::activeSelectionChanged()
{
    setPreviewEnabled(false);
}

void FiltersCategoryModel::filterActivated(int index)
{
    Q_UNUSED(index);
    setPreviewEnabled(false);
}

void FiltersCategoryModel::filterConfigurationChanged(int index, FiltersModel* model)
{
    d->previewFilterID = index;
    if (d->previewEnabled && index > -1)
    {
        if (!model) {
            model = qobject_cast<FiltersModel*>(sender());
        }
        if (!model) {
            return;
        }
        KisFilterConfigurationSP config;
        KisFilter* filter = model->filter(index);
        if (filter->showConfigurationWidget() && filter->id() != QLatin1String("colortransfer")) {
            KisConfigWidget* wdg = filter->createConfigurationWidget(0, d->view->activeNode()->original(), false);
            wdg->deleteLater();
            config = KisFilterConfigurationSP(KisFilterRegistry::instance()->cloneConfiguration(dynamic_cast<KisFilterConfiguration*>(wdg->configuration().data())));
        }
        else {
            config = KisFilterConfigurationSP(KisFilterRegistry::instance()->cloneConfiguration(filter->defaultConfiguration()));
        }
        QObject* configuration = d->categories[d->currentCategory]->configuration(index);
        Q_FOREACH (const QByteArray& propName, configuration->dynamicPropertyNames()) {
            config->setProperty(QString(propName), configuration->property(propName));
        }
        config->setCurve(qobject_cast<PropertyContainer*>(configuration)->curve());
        config->setCurves(qobject_cast<PropertyContainer*>(configuration)->curves());
        configuration->deleteLater();
        d->newConfig = config;
        d->previewTimer->start();
    }
}

void FiltersCategoryModel::updatePreview()
{
    d->view->filterManager()->apply(d->newConfig);
}

bool FiltersCategoryModel::previewEnabled() const
{
    return d->previewEnabled;
}

void FiltersCategoryModel::filterSelected(int index)
{
    if (d->previewEnabled)
        filterConfigurationChanged(index, d->categories[d->currentCategory]);
}

void FiltersCategoryModel::setPreviewEnabled(bool enabled)
{
    if (d->previewEnabled != enabled)
    {
        d->previewEnabled = enabled;
        emit previewEnabledChanged();

        if (enabled)
            filterConfigurationChanged(d->previewFilterID, d->categories[d->currentCategory]);
        else
            d->view->filterManager()->cancel();
    }
}

int FiltersCategoryModel::categoryIndexForConfig(QObject* config)
{
    PropertyContainer* configuration = qobject_cast<PropertyContainer*>(config);
    if (!configuration)
        return -1;
    FiltersModel* model = 0;
    int i = 0;
    while(model == 0 && i < d->categories.count())
    {
        FiltersModel* cat = d->categories.at(i);
        // i know there's no check here - but a category is not created unless there
        // is something to put in it
        for(int j = 0; j < cat->rowCount(); ++j)
        {
            if (cat->filter(j)->id() == configuration->name())
                return i;
        }
        ++i;
    }
    return -1;
}

int FiltersCategoryModel::filterIndexForConfig(int categoryIndex, QObject* filterConfig)
{
    PropertyContainer* configuration = qobject_cast<PropertyContainer*>(filterConfig);
    if (!configuration)
        return -1;
    if (categoryIndex < 0 || categoryIndex > d->categories.count() - 1)
        return -1;
    FiltersModel* cat = d->categories.at(categoryIndex);
    // i know there's no check here - but a category is not created unless there
    // is something to put in it
    for(int j = 0; j < cat->rowCount(); ++j)
    {
        if (cat->filter(j)->id() == configuration->name())
            return j;
    }
    return -1;
}

