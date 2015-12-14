/* This file is part of the KDE project
 * Copyright (C) 2012 Pierre Stirnweiss <pstirnweiss@googlemail.org>
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

#include "StylesFilteredModelBase.h"

#include <KoCharacterStyle.h>
#include <KoParagraphStyle.h>

#include <QDebug>

StylesFilteredModelBase::StylesFilteredModelBase(QObject *parent) 
    : AbstractStylesModel(parent)
    , m_sourceModel(0)
{
}

QModelIndex StylesFilteredModelBase::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column != 0) {
        return QModelIndex();
    }

    if (!parent.isValid()) {
        if (row >= m_proxyToSource.count()) {
            return QModelIndex();
        }
        return createIndex(row, column, int(m_sourceModel->index(m_proxyToSource.at(row), 0, QModelIndex()).internalId()));
    }
    return QModelIndex();
}

QModelIndex StylesFilteredModelBase::parent(const QModelIndex &child) const
{
    Q_UNUSED(child);
    return QModelIndex();
}

int StylesFilteredModelBase::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

int StylesFilteredModelBase::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return m_proxyToSource.size();
    }
    return 0;
}

QVariant StylesFilteredModelBase::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    switch (role) {
    case Qt::DisplayRole: {
        return QVariant();
    }
    case Qt::DecorationRole: {
        return m_sourceModel->data(m_sourceModel->index(m_proxyToSource.at(index.row()), 0, QModelIndex()), role);
        break;
    }
    case Qt::SizeHintRole: {
        return QVariant(QSize(250, 48));
    }
    default: break;
    };
    return QVariant();
}

Qt::ItemFlags StylesFilteredModelBase::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return 0;
    }
    return (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

void StylesFilteredModelBase::setStyleThumbnailer(KoStyleThumbnailer *thumbnailer)
{
    Q_UNUSED(thumbnailer);
}

QModelIndex StylesFilteredModelBase::indexOf(const KoCharacterStyle &style) const
{
    QModelIndex sourceIndex(m_sourceModel->indexOf(style));

    if (!sourceIndex.isValid() || m_sourceToProxy.at(sourceIndex.row()) < 0) {
        return QModelIndex();
    }
    return createIndex(m_sourceToProxy.at(sourceIndex.row()), 0, style.styleId());
}

QImage StylesFilteredModelBase::stylePreview(int row, const QSize &size)
{
    if (row < 0) {
        return QImage();
    }
    return m_sourceModel->stylePreview(m_proxyToSource.at(row), size);

}
/*
QImage StylesFilteredModelBase::stylePreview(QModelIndex &index, const QSize &size)
{
    if (!index.isValid()) {
        return QImage();
    }
    return m_sourceModel->stylePreview(index, size); //TODO be carefull there. this is assuming the sourceModel is only using the internalId, and the index's internalId matches the model's

}
*/
void StylesFilteredModelBase::setStylesModel(AbstractStylesModel *sourceModel)
{
    if (m_sourceModel == sourceModel) {
        return;
    }
    if (m_sourceModel) {
        disconnect(m_sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), this, SLOT(rowsAboutToBeInserted(QModelIndex,int,int)));
        disconnect(m_sourceModel, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
        disconnect(m_sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(m_sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));
        disconnect(m_sourceModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(rowsMoved(QModelIndex,int,int,QModelIndex,int)));
        disconnect(m_sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rowsRemoved(QModelIndex,int,int)));
        disconnect(m_sourceModel, SIGNAL(modelAboutToBeReset()), this, SLOT(modelAboutToBeReset()));
        disconnect(m_sourceModel, SIGNAL(modelReset()), this, SLOT(modelReset()));
    }

    m_sourceModel = sourceModel;
    connect(m_sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), this, SLOT(rowsAboutToBeInserted(QModelIndex,int,int)));
    connect(m_sourceModel, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(m_sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(m_sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(rowsInserted(QModelIndex,int,int)));
    connect(m_sourceModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(rowsMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(m_sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(rowsRemoved(QModelIndex,int,int)));
    connect(m_sourceModel, SIGNAL(modelAboutToBeReset()), this, SLOT(modelAboutToBeReset()));
    connect(m_sourceModel, SIGNAL(modelReset()), this, SLOT(modelReset()));

    beginResetModel();
    createMapping();
    endResetModel();
}

AbstractStylesModel::Type StylesFilteredModelBase::stylesType() const
{
    Q_ASSERT(m_sourceModel);
    return m_sourceModel->stylesType();
}

void StylesFilteredModelBase::modelAboutToBeReset()
{
    beginResetModel();
}

void StylesFilteredModelBase::modelReset()
{
    createMapping();
    endResetModel();
}

void StylesFilteredModelBase::rowsAboutToBeInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    beginResetModel(); //TODO instead of resetting the whole thing, implement proper logic. this will do for a start, there shouldn't be too many styles anyway
}

void StylesFilteredModelBase::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    createMapping();
    endResetModel();
}

void StylesFilteredModelBase::rowsAboutToBeMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destinationParent);
    Q_UNUSED(destinationRow);
    beginResetModel(); //TODO instead of resetting the whole thing, implement proper logic. this will do for a start, there shouldn't be too many styles anyway
}

void StylesFilteredModelBase::rowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destinationParent);
    Q_UNUSED(destinationRow);
    createMapping();
    endResetModel();
}

void StylesFilteredModelBase::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    beginResetModel(); //TODO instead of resetting the whole thing, implement proper logic. this will do for a start, there shouldn't be too many styles anyway
}

void StylesFilteredModelBase::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
    createMapping();
    endResetModel();
}

void StylesFilteredModelBase::createMapping()
{
    Q_ASSERT(m_sourceModel);
    if (!m_sourceModel) {
        return;
    }
    m_sourceToProxy.clear();
    m_proxyToSource.clear();

    for (int i = 0; i < m_sourceModel->rowCount(QModelIndex()); ++i) {
        m_proxyToSource.append(i);
    }

    m_sourceToProxy.fill(-1, m_sourceModel->rowCount(QModelIndex()));
    for (int i = 0; i < m_proxyToSource.count(); ++i) {
        m_sourceToProxy[m_proxyToSource.at(i)] = i;
    }
}
