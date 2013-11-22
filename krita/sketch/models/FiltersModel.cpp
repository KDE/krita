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

#include "FiltersModel.h"
#include <PropertyContainer.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_view2.h>
#include <kis_filter_manager.h>

class FiltersModel::Private
{
public:
    Private()
        : view(0)
    {};
    KisView2* view;
    QList<KisFilterSP> filters;
    QList<KisSafeFilterConfigurationSP> configurations;
};

FiltersModel::FiltersModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private)
{
    QHash<int, QByteArray> roles;
    roles[TextRole] = "text";
    setRoleNames(roles);
}

FiltersModel::~FiltersModel()
{
    delete d;
}

QVariant FiltersModel::data(const QModelIndex& index, int role) const
{
    QVariant data;
    if (index.isValid())
    {
        switch(role)
        {
            case TextRole:
                data = d->filters[index.row()]->name();
                break;
            default:
                break;
        }
    }
    return data;
}

int FiltersModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return d->filters.count();
}

bool FiltersModel::filterRequiresConfiguration(int index)
{
    if (index > -1 && index < d->filters.count())
    {
        return d->filters[index]->showConfigurationWidget();
    }
    return false;
}

QString FiltersModel::filterID(int index)
{
    if (index > -1 && index < d->filters.count())
    {
        return d->filters[index]->id();
    }
    return QLatin1String("");
}

void FiltersModel::activateFilter(int index)
{
    if (index > -1 && index < d->filters.count())
    {
        if (d->configurations[index])
        {
            d->view->filterManager()->apply(d->configurations[index]);
        }
        else
        {
            d->view->filterManager()->apply(KisSafeFilterConfigurationSP(d->filters[index]->defaultConfiguration(d->view->activeNode()->original())));
        }
        d->view->filterManager()->finish();
        emit filterActivated(index);
    }
}

KisFilter* FiltersModel::filter(int index)
{
    if (index > -1 && index < d->filters.count())
    {
        return d->filters[index].data();
    }
    return 0;
}

void FiltersModel::addFilter(KisFilterSP filter)
{
    if (!filter.isNull())
    {
        int newRow = d->filters.count();
        beginInsertRows(QModelIndex(), newRow, newRow);
        d->filters << filter;
        d->configurations << KisSafeFilterConfigurationSP(filter->defaultConfiguration(d->view->activeNode()->original()));
        endInsertRows();
    }
}

QObject* FiltersModel::view() const
{
    return d->view;
}

void FiltersModel::setView(QObject* newView)
{
    d->view = qobject_cast<KisView2*>( newView );
    emit viewChanged();
}

QObject* FiltersModel::configuration(int index)
{
    // If index is out of bounds, return /something/ for the object work on at least.
    if (index < 0 || index > d->configurations.count() - 1)
        return new PropertyContainer("", this);
    QObject* config = new PropertyContainer(d->filters[index]->id(), this);
    // If we've not got one already, assign the default configuration to the cache
    if (!d->configurations[index])
        d->configurations[index] = KisSafeFilterConfigurationSP(d->filters[index]->defaultConfiguration(d->view->activeNode()->original()));
    QMap<QString, QVariant> props = d->configurations[index]->getProperties();
    QMap<QString, QVariant>::const_iterator i;
    for(i = props.constBegin(); i != props.constEnd(); ++i)
    {
        config->setProperty(i.key().toAscii(), i.value());
    }
    return config;
}

void FiltersModel::setConfiguration(int index, QObject* configuration)
{
    if (configuration && index > -1 && index < d->configurations.count() - 1)
    {
        KisSafeFilterConfigurationSP config = d->configurations[index];
        foreach(const QByteArray& propName, configuration->dynamicPropertyNames())
        {
            config->setProperty(QString(propName), configuration->property(propName));
        }
        d->configurations[index] = config;
        emit configurationChanged(index);
    }
}

#include "FiltersModel.moc"
