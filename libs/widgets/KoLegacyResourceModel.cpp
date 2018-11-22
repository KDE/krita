/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
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

#include "KoLegacyResourceModel.h"

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include <KoResourceServerAdapter.h>
#include <math.h>

KoLegacyResourceModel::KoLegacyResourceModel(QSharedPointer<KoAbstractResourceServerAdapter> resourceAdapter, QObject * parent)
    : QAbstractTableModel(parent)
    , m_resourceAdapter(resourceAdapter)
    , m_columnCount(4)
{
    Q_ASSERT(m_resourceAdapter);
    m_resourceAdapter->connectToResourceServer();
    connect(m_resourceAdapter.data(), SIGNAL(resourceAdded(KoResourceSP)),
            this, SLOT(resourceAdded(KoResourceSP)));
    connect(m_resourceAdapter.data(), SIGNAL(removingResource(KoResourceSP)),
            this, SLOT(resourceRemoved(KoResourceSP)));
    connect(m_resourceAdapter.data(), SIGNAL(resourceChanged(KoResourceSP)),
            this, SLOT(resourceChanged(KoResourceSP)));
    connect(m_resourceAdapter.data(), SIGNAL(tagsWereChanged()),
            this, SLOT(tagBoxEntryWasModified()));
    connect(m_resourceAdapter.data(), SIGNAL(tagCategoryWasAdded(QString)),
            this, SLOT(tagBoxEntryWasAdded(QString)));
    connect(m_resourceAdapter.data(), SIGNAL(tagCategoryWasRemoved(QString)),
            this, SLOT(tagBoxEntryWasRemoved(QString)));
}

KoLegacyResourceModel::~KoLegacyResourceModel()
{
    if (!m_currentTag.isEmpty()) {
        KConfigGroup group =  KSharedConfig::openConfig()->group("SelectedTags");
        group.writeEntry(serverType(), m_currentTag);
    }

}

int KoLegacyResourceModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    int resourceCount = m_resourceAdapter->resources().count();
    if (!resourceCount)
        return 0;

    return static_cast<int>(ceil(static_cast<qreal>(resourceCount) / m_columnCount));
}

int KoLegacyResourceModel::columnCount ( const QModelIndex & ) const
{
    return m_columnCount;
}

QVariant KoLegacyResourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    int i = index.row() * m_columnCount + index.column();
    const QList<KoResourceSP> resources = m_resourceAdapter->resources();
    if (i >= resources.count() || i < 0) {
        return QVariant();
    }

    KoResourceSP resource = resources[i];
    if (!resource) {
        return QVariant();
    }

    switch (role)
    {
        case Qt::DisplayRole:
        {
            QString resName = i18n( resource->name().toUtf8().data());
            return QVariant( resName );
        }
        case KoLegacyResourceModel::TagsRole:
        {
            if (m_resourceAdapter->assignedTagsList(resource).count()) {
                QString taglist = m_resourceAdapter->assignedTagsList(resource).join("</li><li>");
                return QString("<li>%2</li>").arg(taglist);
            } else {
                return QString();
            }
        }
        case Qt::DecorationRole:
        {
            return QVariant( resource->image() );
        }
        case KoLegacyResourceModel::LargeThumbnailRole:
        {
            QSize imageSize = resource->image().size();
            QSize thumbSize( 100, 100 );
            if(imageSize.height() > thumbSize.height() || imageSize.width() > thumbSize.width()) {
                qreal scaleW = static_cast<qreal>( thumbSize.width() ) / static_cast<qreal>( imageSize.width() );
                qreal scaleH = static_cast<qreal>( thumbSize.height() ) / static_cast<qreal>( imageSize.height() );

                qreal scale = qMin( scaleW, scaleH );

                int thumbW = static_cast<int>( imageSize.width() * scale );
                int thumbH = static_cast<int>( imageSize.height() * scale );

                return QVariant(resource->image().scaled( thumbW, thumbH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            }
            else
                return QVariant(resource->image());
        }

        default:
            return QVariant();
    }
}

QModelIndex KoLegacyResourceModel::index ( int row, int column, const QModelIndex & ) const
{
    int index = row * m_columnCount + column;
    const QList<KoResourceSP> resources = m_resourceAdapter->resources();
    if (index >= resources.count() || index < 0) {
        return QModelIndex();
    }

    return createIndex(row, column);
}

void KoLegacyResourceModel::doSafeLayoutReset(KoResourceSP activateAfterReformat)
{
    emit beforeResourcesLayoutReset(activateAfterReformat);
    beginResetModel();
    endResetModel();
    emit afterResourcesLayoutReset();
}

void KoLegacyResourceModel::setColumnCount( int columnCount )
{
    if (columnCount != m_columnCount) {
        emit beforeResourcesLayoutReset(0);
        m_columnCount = columnCount;
        beginResetModel();
        endResetModel();
        emit afterResourcesLayoutReset();
    }
}

void KoLegacyResourceModel::resourceAdded(KoResourceSP resource)
{
    int newIndex = m_resourceAdapter->resources().indexOf(resource);
    if (newIndex >= 0) {
        doSafeLayoutReset(0);
    }
}

void KoLegacyResourceModel::resourceRemoved(KoResourceSP resource)
{
    Q_UNUSED(resource);
    doSafeLayoutReset(0);
}

void KoLegacyResourceModel::resourceChanged(KoResourceSP resource)
{
    int resourceIndex = m_resourceAdapter->resources().indexOf(resource);
    int row = resourceIndex / columnCount();
    int column = resourceIndex % columnCount();

    QModelIndex modelIndex = index(row, column);
    if (!modelIndex.isValid()) {
        return;
    }

    emit dataChanged(modelIndex, modelIndex);
}

void KoLegacyResourceModel::tagBoxEntryWasModified()
{
    m_resourceAdapter->updateServer();
    emit tagBoxEntryModified();
}

void KoLegacyResourceModel::tagBoxEntryWasAdded(const QString& tag)
{
    emit tagBoxEntryAdded(tag);
}

void KoLegacyResourceModel::tagBoxEntryWasRemoved(const QString& tag)
{
    emit tagBoxEntryRemoved(tag);
}

QModelIndex KoLegacyResourceModel::indexFromResource(KoResourceSP resource) const
{
    int resourceIndex = m_resourceAdapter->resources().indexOf(resource);
    if (columnCount() > 0) {
        int row = resourceIndex / columnCount();
        int column = resourceIndex % columnCount();
        return index(row, column);
    }
    return QModelIndex();
}

QString KoLegacyResourceModel::extensions() const
{
    return m_resourceAdapter->extensions();
}

void KoLegacyResourceModel::importResourceFile(const QString &filename)
{
    m_resourceAdapter->importResourceFile(filename);
}

void KoLegacyResourceModel::importResourceFile(const QString & filename, bool fileCreation)
{
    m_resourceAdapter->importResourceFile(filename, fileCreation);
}

bool KoLegacyResourceModel::removeResource(KoResourceSP resource)
{
    return m_resourceAdapter->removeResource(resource);
}

void KoLegacyResourceModel::removeResourceFile(const QString &filename)
{
    m_resourceAdapter->removeResourceFile(filename);
}

QStringList KoLegacyResourceModel::assignedTagsList(KoResourceSP resource) const
{
    return m_resourceAdapter->assignedTagsList(resource);
}

void KoLegacyResourceModel::addTag(KoResourceSP resource, const QString& tag)
{
    m_resourceAdapter->addTag(resource, tag);
    emit tagBoxEntryAdded(tag);
}

void KoLegacyResourceModel::deleteTag(KoResourceSP resource, const QString &tag)
{
    m_resourceAdapter->deleteTag(resource, tag);
}

QStringList KoLegacyResourceModel::tagNamesList() const
{
    return m_resourceAdapter->tagNamesList();
}

QStringList KoLegacyResourceModel::searchTag(const QString& lineEditText)
{
    return m_resourceAdapter->searchTag(lineEditText);
}

void KoLegacyResourceModel::searchTextChanged(const QString& searchString)
{
    m_resourceAdapter->searchTextChanged(searchString);
}

void KoLegacyResourceModel::enableResourceFiltering(bool enable)
{
    m_resourceAdapter->enableResourceFiltering(enable);
}

void KoLegacyResourceModel::setCurrentTag(const QString& currentTag)
{
    m_currentTag = currentTag;
    m_resourceAdapter->setCurrentTag(currentTag);
}

void KoLegacyResourceModel::updateServer()
{
    m_resourceAdapter->updateServer();
}

int KoLegacyResourceModel::resourcesCount() const
{
    return m_resourceAdapter->resources().count();
}

QList<KoResourceSP > KoLegacyResourceModel::currentlyVisibleResources() const
{
  return m_resourceAdapter->resources();
}

void KoLegacyResourceModel::tagCategoryMembersChanged()
{
    m_resourceAdapter->tagCategoryMembersChanged();
}

void KoLegacyResourceModel::tagCategoryAdded(const QString& tag)
{
    m_resourceAdapter->tagCategoryAdded(tag);
}

void KoLegacyResourceModel::tagCategoryRemoved(const QString& tag)
{
    m_resourceAdapter->tagCategoryRemoved(tag);
}

QString KoLegacyResourceModel::serverType() const
{
    return m_resourceAdapter->serverType();
}

QList< KoResourceSP > KoLegacyResourceModel::serverResources() const
{
    return m_resourceAdapter->serverResources();
}
