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

#ifndef KORESOURCEMODEL_H
#define KORESOURCEMODEL_H

#include <QAbstractTableModel>

class KoAbstractResourceServerAdapter;
class KoResource;

/// The resource model managing the resource data
class KoResourceModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit KoResourceModel( KoAbstractResourceServerAdapter * resourceAdapter, QObject * parent = 0 );
    virtual ~KoResourceModel() {}

    /// reimplemented
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    /// reimplemented
    virtual int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    /// reimplemented
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    /// reimplemented
    virtual QModelIndex index ( int row, int column = 0, const QModelIndex & parent = QModelIndex() ) const;
    /// Sets the number of columns to display
    void setColumnCount( int columnCount );

    /// Extensions to Qt::ItemDataRole.
    enum ItemDataRole
    {
        /// A larger thumbnail for displaying in a tooltip. 200x200 or so.
        LargeThumbnailRole = 33
    };

    QModelIndex indexFromResource(KoResource* resource) const;

    /// facade for KoAbstractResourceServerAdapter
    QString extensions() const;
    void importResourceFile(const QString &filename);
    void importResourceFile(const QString &filename, bool fileCreation);
    bool removeResource(KoResource* resource);
    void removeResourceFile(const QString & filename);
    QStringList assignedTagsList(KoResource *resource) const;
    void addTag(KoResource* resource, const QString& tag);
    void deleteTag( KoResource* resource, const QString& tag);
    QStringList tagNamesList() const;
    QStringList searchTag(const QString& lineEditText);
    void enableResourceFiltering(bool enable);
    void setCurrentTag(const QString& currentTag);
    void searchTextChanged(const QString& searchString);
    void updateServer();
    int resourcesCount() const;
    QList<KoResource *> currentlyVisibleResources() const;
    QList<KoResource *> serverResources() const;
    void tagCategoryMembersChanged();
    void tagCategoryAdded(const QString& tag);
    void tagCategoryRemoved(const QString& tag);

signals:
    /// XXX: not sure if this is the best place for these
    void tagBoxEntryModified();
    void tagBoxEntryAdded(const QString& tag);
    void tagBoxEntryRemoved(const QString& tag);

private slots:
    void resourceAdded(KoResource *resource);
    void resourceRemoved(KoResource *resource);
    void resourceChanged(KoResource *resource);
    void tagBoxEntryWasModified();
    void tagBoxEntryWasAdded(const QString& tag);
    void tagBoxEntryWasRemoved(const QString& tag);

private:
    KoAbstractResourceServerAdapter *m_resourceAdapter;
    int m_columnCount;
};

#endif // KORESOURCEMODEL_H
