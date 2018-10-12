/* This file is part of the KDE project
 *
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KO_GENERIC_REGISTRY_MODEL_H_
#define _KO_GENERIC_REGISTRY_MODEL_H_

#include <QAbstractListModel>
#include "KoGenericRegistry.h"

/**
 * This is a model that you can use to display the content of a registry.
 *
 * @param T is the type of the data in the registry
 */
template<typename T>
class KoGenericRegistryModel : public QAbstractListModel
{

public:

    KoGenericRegistryModel(KoGenericRegistry<T>* registry);

    ~KoGenericRegistryModel() override;

public:

    /**
     * @return the number of elements in the registry
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * When role == Qt::DisplayRole, this function will return the name of the
     * element.
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    /**
     * @return the element at the given index
     */
    T get(const QModelIndex &index) const;

private:

    KoGenericRegistry<T>* m_registry;
};

// -- Implementation --
template<typename T>
KoGenericRegistryModel<T>::KoGenericRegistryModel(KoGenericRegistry<T>* registry) : m_registry(registry)
{
}

template<typename T>
KoGenericRegistryModel<T>::~KoGenericRegistryModel()
{
}

template<typename T>
int KoGenericRegistryModel<T>::rowCount(const QModelIndex &/*parent*/) const
{
    return m_registry->keys().size();
}

template<typename T>
QVariant KoGenericRegistryModel<T>::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return QVariant(get(index)->name());
    }
    return QVariant();
}

template<typename T>
T KoGenericRegistryModel<T>::get(const QModelIndex &index) const
{
    return m_registry->get(m_registry->keys()[index.row()]);
}

#endif
