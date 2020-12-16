/* This file is part of the KDE project
 *
 *  SPDX-FileCopyrightText: 2007 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
