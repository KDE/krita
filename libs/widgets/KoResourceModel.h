/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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
    /// @returns the resource server adapter the model is connnected to
    KoAbstractResourceServerAdapter * resourceServerAdapter();

    /// Extensions to Qt::ItemDataRole.
    enum ItemDataRole
    {
        /// A larger thumbnail for displaying in a tooltip. 200x200 or so.
        LargeThumbnailRole = 33
    };

private slots:
    void resourceAdded(KoResource *resource);
    void resourceRemoved(KoResource *resource);
    void resourceChanged(KoResource *resource);

private:
    KoAbstractResourceServerAdapter * m_resourceAdapter;
    int m_columnCount;
};

#endif // KORESOURCEMODEL_H
