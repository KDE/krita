/*
 * SPDX-FileCopyrightText: 2018 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
        return m_index.data(Qt::UserRole + KisAbstractResourceModel::Id).toInt();
    }
    return -1;
}

QString KisResourceItem::resourceType()
{
    if (m_index.isValid()) {
        return m_index.data(Qt::UserRole + KisAbstractResourceModel::ResourceType).toString();
    }
    return QString();
}

QString KisResourceItem::name()
{
    if (m_index.isValid()) {
        return m_index.data(Qt::UserRole + KisAbstractResourceModel::Name).toString();
    }
    return QString();
}

QString KisResourceItem::filename()
{
    if (m_index.isValid()) {
        return m_index.data(Qt::UserRole + KisAbstractResourceModel::Filename).toString();
    }
    return QString();
}

QString KisResourceItem::tooltip()
{
    if (m_index.isValid()) {
        return m_index.data(Qt::UserRole + KisAbstractResourceModel::Tooltip).toString();
    }
    return QString();
}

QByteArray KisResourceItem::md5()
{
    return resource()->md5();
}

QImage KisResourceItem::thumbnail()
{
    if (m_index.isValid()) {
        return m_index.data(Qt::UserRole + KisAbstractResourceModel::Thumbnail).value<QImage>();
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
    return d->currentRow < d->resourceModel->rowCount();
}

bool KisResourceIterator::hasPrevious() const
{
    return d->currentRow > 0 && d->resourceModel->rowCount() > 0;
}

const KisResourceItemSP KisResourceIterator::next()
{
    if (hasNext()) {
        QModelIndex idx = d->resourceModel->index(d->currentRow, 0);
        d->currentRow++;
        return KisResourceItemSP(new KisResourceItem(d->resourceModel, idx));
    }
    return KisResourceItemSP(new KisResourceItem(0, QModelIndex()));
}

const KisResourceItemSP KisResourceIterator::peekNext() const
{
    if (hasNext()) {
        QModelIndex idx = d->resourceModel->index(d->currentRow, 0);
        return KisResourceItemSP(new KisResourceItem(d->resourceModel, idx));
    }
    return KisResourceItemSP(new KisResourceItem(0, QModelIndex()));
}

const KisResourceItemSP KisResourceIterator::peekPrevious() const
{
    if (hasPrevious()) {
        QModelIndex idx = d->resourceModel->index(d->currentRow - 1, 0);
        return KisResourceItemSP(new KisResourceItem(d->resourceModel, idx));
    }
    return KisResourceItemSP(new KisResourceItem(0, QModelIndex()));
}

const KisResourceItemSP KisResourceIterator::previous()
{
    if (hasPrevious()) {
        d->currentRow--;
        QModelIndex idx = d->resourceModel->index(d->currentRow, 0);
        return KisResourceItemSP(new KisResourceItem(d->resourceModel, idx));
    }
    return KisResourceItemSP(new KisResourceItem(0, QModelIndex()));

}

void KisResourceIterator::toFront()
{
    d->currentRow = 0;
}

void KisResourceIterator::toBack()
{
    d->currentRow = d->resourceModel->rowCount();
}
