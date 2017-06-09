/* This file is part of the KDE project
 * Copyright (C) 2014 Victor Lafon <metabolic.ewilan@hotmail.fr>
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

#ifndef KORESOURCEMODELBASE_H
#define KORESOURCEMODELBASE_H

#include <QAbstractTableModel>
#include "kritawidgets_export.h"

class KoResource;

/// The resource model managing the resource data
class KRITAWIDGETS_EXPORT KoResourceModelBase : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit KoResourceModelBase(QObject * parent = 0 );
    ~KoResourceModelBase() override;

    /// reimplemented
    int rowCount(const QModelIndex &parent = QModelIndex()) const override =0;
    /// reimplemented
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const override =0;
    /// reimplemented
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override =0;
    /// reimplemented
    QModelIndex index ( int row, int column = 0, const QModelIndex & parent = QModelIndex() ) const override =0;

    virtual QModelIndex indexFromResource(KoResource* resource) const =0;
    virtual QStringList assignedTagsList(KoResource *resource) const =0;
    virtual bool removeResource(KoResource* resource) =0;
    virtual void addTag(KoResource* resource, const QString& tag) =0;
    virtual void deleteTag(KoResource *resource, const QString &tag) =0;
    virtual int resourcesCount() const =0;
    virtual QList<KoResource *> currentlyVisibleResources() const =0;
    virtual void updateServer() =0;
    virtual void enableResourceFiltering(bool enable) =0;
    virtual void searchTextChanged(const QString& searchString) =0;
    virtual QStringList tagNamesList() const =0;
    virtual void setCurrentTag(const QString& currentTag) =0;
    virtual QList<KoResource *> serverResources() const =0;
    virtual void tagCategoryMembersChanged() =0;
    virtual void tagCategoryAdded(const QString& tag) =0;
    virtual void tagCategoryRemoved(const QString& tag) =0;

private Q_SLOTS:
    virtual void tagBoxEntryWasModified() =0;
    virtual void tagBoxEntryWasAdded(const QString& tag) =0;
    virtual void tagBoxEntryWasRemoved(const QString& tag) =0;
    virtual void resourceAdded(KoResource *resource) =0;
    virtual void resourceRemoved(KoResource *resource) =0;
    virtual void resourceChanged(KoResource* resource) =0;
};
#endif // KORESOURCEMODELBASE_H

