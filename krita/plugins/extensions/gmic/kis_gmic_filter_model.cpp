/*
 * Copyright (c) 2013 Lukáš Tvrdý <lukast.dev@gmail.com
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

#include <kis_gmic_filter_model.h>
#include "Component.h"
#include <Filters.h>
#include "Command.h"

#include "kis_gmic_filter_settings.h"
#include "kis_gmic_settings_widget.h"

// debug
#include <typeinfo>
#include <iostream>

KisGmicFilterModel::KisGmicFilterModel(Component * rootComponent, QObject* parent):
    QAbstractItemModel(parent),
    m_rootComponent(rootComponent)
{
    //m_rootComponent->print();
}

KisGmicFilterModel::~KisGmicFilterModel()
{

}


QModelIndex KisGmicFilterModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)){
         return QModelIndex();
    }

    Component * parentItem;
    if (!parent.isValid())
    {
        parentItem = m_rootComponent;
    } else
    {
        parentItem = static_cast<Component*>(parent.internalPointer());
    }

    Component *childItem = parentItem->child(row);
    if (childItem) {
        return createIndex(row, column, childItem);
    }
    else
    {
        return QModelIndex();
    }

}

QModelIndex KisGmicFilterModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    Component *childItem = static_cast<Component*>(index.internalPointer());
    Component *parentItem = childItem->parent();

    if (parentItem == 0)
    {
        return QModelIndex();
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int KisGmicFilterModel::rowCount(const QModelIndex& parent) const
 {
     Component *parentItem;
     if (parent.column() > 0)
         return 0;

     if (!parent.isValid()){
         parentItem = m_rootComponent;
     }
     else
     {
         parentItem = static_cast<Component*>(parent.internalPointer());
     }

     return parentItem->childCount();
 }

int KisGmicFilterModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
         return static_cast<Component*>(parent.internalPointer())->columnCount();
    }
    else
    {
         return m_rootComponent->columnCount();
    }
}

QVariant KisGmicFilterModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        Component *item = static_cast<Component*>(index.internalPointer());
        return item->data(index.column());
    }

    if (role == WidgetRole)
    {
        Component *item = static_cast<Component*>(index.internalPointer());
        Command * commandItem = dynamic_cast<Command *>(item);
        if (commandItem)
        {
            //KisGmicSettingsWidget * filterSettingsWidget = new  KisGmicSettingsWidget(commandItem);
            /*if (filterSettingsWidget)
            {
                std::cout << "Sending QWidget" << std::endl;
                return QVariant::fromValue(filterSettingsWidget);
            }*/
            return QVariant::fromValue(commandItem);
        }
    }

    if (role == FilterSettingsRole)
    {
        Component *item = static_cast<Component*>(index.internalPointer());
        Command * commandItem = dynamic_cast<Command *>(item);
        if (commandItem)
        {
            KisGmicFilterSetting * settings = new KisGmicFilterSetting;
            commandItem->writeConfiguration(settings);
            return QVariant::fromValue<KisGmicFilterSetting *>(settings);
        }

    }


    return QVariant();
}


Qt::ItemFlags KisGmicFilterModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
    {
         return 0;
    }


    Component *item = static_cast<Component*>(index.internalPointer());
    Command * commandItem = dynamic_cast<Command *>(item);
    if (commandItem)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;;
}

QVariant KisGmicFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    return m_rootComponent->name();
}


/*bool KisGmicFilterModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::EditRole)
    {
        Component *item = static_cast<Component*>(index.internalPointer());
        Command * commandItem = dynamic_cast<Command *>(item);
        if (commandItem)
        {
            //TODO
        }


    }
}
*/
