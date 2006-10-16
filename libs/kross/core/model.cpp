/***************************************************************************
 * model.cpp
 * This file is part of the KDE project
 * copyright (C) 2006 by Sebastian Sauer (mail@dipe.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * You should have received a copy of the GNU Library General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "model.h"
#include "action.h"
#include "manager.h"

#include <QEvent>

#include <kicon.h>
#include <kmenu.h>

using namespace Kross;

/******************************************************************************
 * ActionCollectionModel
 */

namespace Kross {

    /// \internal d-pointer class.
    class ActionCollectionModel::Private
    {
        public:
            KActionCollection* actioncollection;
    };

}

ActionCollectionModel::ActionCollectionModel(QObject* parent)
    : QAbstractItemModel(parent)
    , d( new Private() )
{
    d->actioncollection = Manager::self().actionCollection();
}

ActionCollectionModel::~ActionCollectionModel()
{
    delete d;
}

int ActionCollectionModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int ActionCollectionModel::rowCount(const QModelIndex&) const
{
    return d->actioncollection->actions(QString::null).count();
}

QModelIndex ActionCollectionModel::index(int row, int column, const QModelIndex& parent) const
{
    Action* action = dynamic_cast< Action* >( d->actioncollection->actions().value(row) );
    if( ! action || parent.isValid() )
        return QModelIndex();
    return createIndex(row, column, action);
}

QModelIndex ActionCollectionModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

Qt::ItemFlags ActionCollectionModel::flags(const QModelIndex &index) const
{
    if( ! index.isValid() )
        return Qt::ItemIsEnabled;
    //if(index.column() == 0 /*&& d->editable*/) return QAbstractItemModel::flags(index); // | Qt::ItemIsUserCheckable;
    return QAbstractItemModel::flags(index); // | Qt::ItemIsEditable;
}

QVariant ActionCollectionModel::data(const QModelIndex& index, int role) const
{
    if( ! index.isValid() )
        return QVariant();
    Action* action = static_cast< Action* >( index.internalPointer() );
    switch( role ) {
        case Qt::DecorationRole:
            return action->icon();
        case Qt::DisplayRole:
            return action->text().replace("&","");
        case Qt::ToolTipRole: // fall through
        case Qt::WhatsThisRole:
            return action->description();
        //case Qt::CheckStateRole:
        //    return action->isVisible();
        default:
            return QVariant();
    }
}

bool ActionCollectionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if( ! index.isValid() /*|| ! d->editable*/ )
        return false;
    Action* action = static_cast< Action* >( index.internalPointer() );
    switch( role ) {
        case Qt::EditRole: {
            action->setText( value.toString() );
        } break;
        case Qt::CheckStateRole: {
            action->setVisible( ! action->isVisible() );
        } break;
        default:
            return false;
    }
    emit dataChanged(index, index);
    return true;
}

/******************************************************************************
 * ActionCollectionProxyModel
 */

ActionCollectionProxyModel::ActionCollectionProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    setSourceModel( new ActionCollectionModel(this) );
}

ActionCollectionProxyModel::~ActionCollectionProxyModel()
{
}

void ActionCollectionProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);
}

/*
bool ActionCollectionProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(index.row(), 0, source_parent);
    if( ! index.isValid() )
        return false;
    Action* action = static_cast< Action* >( index.internalPointer() );
    return action->isEnabled();
}
*/

#if 0

/******************************************************************************
 * ActionMenuModel
 */

namespace Kross {

    /// \internal d-pointer class.
    class ActionMenuModel::Private
    {
        public:
            KMenu* menu;
    };

}

ActionMenuModel::ActionMenuModel(QObject* parent, KMenu* menu)
    : QAbstractItemModel(parent)
    , d( new Private() )
{
    d->menu = menu;
}

ActionMenuModel::~ActionMenuModel()
{
    delete d;
}

int ActionMenuModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int ActionMenuModel::rowCount(const QModelIndex&) const
{
    return d->menu->actions().count();
}

QModelIndex ActionMenuModel::index(int row, int column, const QModelIndex& parent) const
{
    if(parent.isValid())
        return QModelIndex();
    return createIndex(row, column, d->menu->actions().value(row));
}

QModelIndex ActionMenuModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

QVariant ActionMenuModel::data(const QModelIndex& index, int role) const
{
    if(! index.isValid())
        return QVariant();
    KAction* action = static_cast< KAction* >( index.internalPointer() );
    switch( role ) {
        case Qt::DecorationRole:
            return action->icon();
        case Qt::DisplayRole:
            return action->text().replace("&","");
        case Qt::ToolTipRole: // fall through
        case Qt::WhatsThisRole: {
            Action* a = dynamic_cast< Action* >(action);
            return a ? a->description() : QVariant();
        }
        default:
            return QVariant();
    }
}

#endif
