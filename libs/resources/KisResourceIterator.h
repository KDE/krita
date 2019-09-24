/*
 * Copyright (C) 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KISRESOURCEITERATOR_H
#define KISRESOURCEITERATOR_H

#include <QImage>
#include <QString>
#include <QScopedPointer>
#include <QModelIndex>

#include <KoResource.h>

class KisResourceModel;

#include <kritaresources_export.h>

/**
 * @brief The KisResourceItem class represents a resource, but until resource() is called,
 * the resource is not loaded; the rest of the information comes from the cache database.
 */
class KRITARESOURCES_EXPORT KisResourceItem {
private:
    friend class KisResourceIterator;
    KisResourceItem(KisResourceModel *resourceModel, const QModelIndex &index);
public:
    int id();
    QString resourceType();
    QString name();
    QString filename();
    QString tooltip();
    QImage thumbnail();
    KoResourceSP resource();
private:
    KisResourceModel *m_resourceModel;
    QModelIndex m_index;
};

typedef QSharedPointer<KisResourceItem> KisResourceItemSP;

/**
 * @brief The KisResourceIterator class provides an iterator
 * for a KisResourceModel.
 */
class KRITARESOURCES_EXPORT KisResourceIterator
{
public:
    KisResourceIterator(KisResourceModel *resourceModel);
    ~KisResourceIterator();

    bool hasNext() const;
    bool hasPrevious() const;
    const KisResourceItemSP next();
    const KisResourceItemSP peekNext() const;
    const KisResourceItemSP peekPrevious() const;
    const KisResourceItemSP previous();
    void toBack();
    void toEnd();

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif // KisResourceITERATOR_H
