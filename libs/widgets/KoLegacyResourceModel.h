/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOLEGACYRESOURCEMODEL_H
#define KOLEGACYRESOURCEMODEL_H

#include <QSharedPointer>
#include "QAbstractTableModel"

#include <KoResource.h>

#include "kritawidgets_export.h"

class KoAbstractResourceServerAdapter;
class KoResource;

/// The resource model managing the resource data
class KRITAWIDGETS_EXPORT KoLegacyResourceModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit KoLegacyResourceModel(QSharedPointer<KoAbstractResourceServerAdapter> resourceAdapter, QObject * parent = 0);
    ~KoLegacyResourceModel() override;

    /// reimplemented
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /// reimplemented
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const override;
    /// reimplemented
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /// reimplemented
    QModelIndex index ( int row, int column = 0, const QModelIndex & parent = QModelIndex() ) const override;
    /// Sets the number of columns to display
    void setColumnCount( int columnCount );

    /// Extensions to Qt::ItemDataRole.
    enum ItemDataRole
    {
        /// A larger thumbnail for displaying in a tooltip. 200x200 or so.
        LargeThumbnailRole = Qt::UserRole + 1,
        TagsRole
    };

    QModelIndex indexFromResource(KoResourceSP resource) const;

    /// facade for KoAbstractResourceServerAdapter
    QString extensions() const;
    void importResourceFile(const QString &filename);
    void importResourceFile(const QString &filename, bool fileCreation);
    bool removeResource(KoResourceSP resource);
    void removeResourceFile(const QString & filename);
    QStringList assignedTagsList(KoResourceSP resource) const;
    void addTag(KoResourceSP resource, const QString& tag);
    void deleteTag( KoResourceSP resource, const QString& tag);
    QStringList tagNamesList() const;
    QStringList searchTag(const QString& lineEditText);
    void enableResourceFiltering(bool enable);
    void setCurrentTag(const QString& currentTag);
    void searchTextChanged(const QString& searchString);
    void updateServer();
    int resourcesCount() const;
    QList<KoResourceSP > currentlyVisibleResources() const;
    QList<KoResourceSP > serverResources() const;
    void tagCategoryMembersChanged();
    void tagCategoryAdded(const QString& tag);
    void tagCategoryRemoved(const QString& tag);

    QString serverType() const;

Q_SIGNALS:
    /// XXX: not sure if this is the best place for these
    void tagBoxEntryModified();
    void tagBoxEntryAdded(const QString& tag);
    void tagBoxEntryRemoved(const QString& tag);

    void beforeResourcesLayoutReset(KoResourceSP activateAfterReformat);
    void afterResourcesLayoutReset();

private:
    void doSafeLayoutReset(KoResourceSP activateAfterReformat);

private Q_SLOTS:
    void resourceAdded(KoResourceSP resource);
    void resourceRemoved(KoResourceSP resource);
    void resourceChanged(KoResourceSP resource);
    void tagBoxEntryWasModified();
    void tagBoxEntryWasAdded(const QString& tag);
    void tagBoxEntryWasRemoved(const QString& tag);

private:
    QSharedPointer<KoAbstractResourceServerAdapter> m_resourceAdapter;
    int m_columnCount;
    QString m_currentTag;
};

#endif // KoLegacyResourceModel_H
