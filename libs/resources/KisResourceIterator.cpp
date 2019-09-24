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
#include "KisResourceIterator.h"

#include <KisResourceModel.h>
#include <QModelIndex>

KisResourceItem::KisResourceItem(KisResourceModel *resourceModel, const QModelIndex &index)
    : m_resourceModel(resourceModel)
    , m_index(index)
{

}

int KisResourceItem::id()
{
    if (m_index.isValid()) {
        return m_index.data(Qt::UserRole + KisResourceModel::Id).toInt();
    }
    return -1;
}

QString KisResourceItem::resourceType()
{
    if (m_index.isValid()) {
        return m_index.data(Qt::UserRole + KisResourceModel::ResourceType).toString();
    }
    return QString();
}

QString KisResourceItem::name()
{
    if (m_index.isValid()) {
        return m_index.data(Qt::UserRole + KisResourceModel::Name).toString();
    }
    return QString();
}

QString KisResourceItem::filename()
{
    if (m_index.isValid()) {
        return m_index.data(Qt::UserRole + KisResourceModel::Filename).toString();
    }
    return QString();
}

QString KisResourceItem::tooltip()
{
    if (m_index.isValid()) {
        return m_index.data(Qt::UserRole + KisResourceModel::Tooltip).toString();
    }
    return QString();
}

QImage KisResourceItem::thumbnail()
{
    if (m_index.isValid()) {
        return m_index.data(Qt::UserRole + KisResourceModel::Image).value<QImage>();
    }
    return QImage();
}

KoResourceSP KisResourceItem::resource()
{
    if (m_index.isValid() && m_resourceModel) {
       return m_resourceModel->resourceForIndex(m_index);
    }
    return 0;
}

struct KisResourceIterator::Private
{
    Private(KisResourceModel *_resourceModel)
        : resourceModel(_resourceModel)
    {}

    KisResourceModel *resourceModel {0};
    int currentRow {0};
};

KisResourceIterator::KisResourceIterator(KisResourceModel *resourceModel)
    : d(new Private(resourceModel))
{
}

KisResourceIterator::~KisResourceIterator()
{
}


bool KisResourceIterator::hasNext() const
{
    return d->currentRow < d->resourceModel->rowCount() - 1;
}

bool KisResourceIterator::hasPrevious() const
{
    return d->currentRow > 0 && d->resourceModel->rowCount() > 1 && d->currentRow < d->resourceModel->rowCount();
}

const KisResourceItemSP KisResourceIterator::next()
{
    if (d->currentRow < d->resourceModel->rowCount() - 1) {
        d->currentRow++;
        QModelIndex idx = d->resourceModel->index(d->currentRow, 0);
        return KisResourceItemSP(new KisResourceItem(d->resourceModel, idx));
    }
    return KisResourceItemSP(new KisResourceItem(0, QModelIndex()));
}

const KisResourceItemSP KisResourceIterator::peekNext() const
{
    if (d->currentRow < d->resourceModel->rowCount() - 2) {
        QModelIndex idx = d->resourceModel->index(d->currentRow + 1, 0);
        return KisResourceItemSP(new KisResourceItem(d->resourceModel, idx));
    }
    return KisResourceItemSP(new KisResourceItem(0, QModelIndex()));
}

const KisResourceItemSP KisResourceIterator::peekPrevious() const
{
    if (d->currentRow > 1 && d->resourceModel->rowCount() > 2) {
        QModelIndex idx = d->resourceModel->index(d->currentRow -1, 0);
        return KisResourceItemSP(new KisResourceItem(d->resourceModel, idx));
    }
    return KisResourceItemSP(new KisResourceItem(0, QModelIndex()));
}

const KisResourceItemSP KisResourceIterator::previous()
{
    if (d->currentRow > 1 && d->resourceModel->rowCount() > 2) {
        d->currentRow--;
        QModelIndex idx = d->resourceModel->index(d->currentRow, 0);
        return KisResourceItemSP(new KisResourceItem(d->resourceModel, idx));
    }
    return KisResourceItemSP(new KisResourceItem(0, QModelIndex()));

}

void KisResourceIterator::toBack()
{
    d->currentRow = 0;
}

void KisResourceIterator::toEnd()
{
    d->currentRow = d->resourceModel->rowCount() -1;
}
