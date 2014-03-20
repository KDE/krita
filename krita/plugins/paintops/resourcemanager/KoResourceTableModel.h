/*
 *  Copyright (c) 2014 Victor Lafon metabolic.ewilan@hotmail.fr
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

#ifndef KORESOURCETABLEMODEL_H
#define KORESOURCETABLEMODEL_H

#include "KoResourceServerAdapter.h"
#include "KoResourceModelBase.h"
#include <QModelIndex>

class KoResource;

class KoResourceTableModel : public KoResourceModelBase
{
    Q_OBJECT
    public:
        KoResourceTableModel(QList<KoAbstractResourceServerAdapter*> resourceAdapterList, QObject * parent = 0 );
        int rowCount(const QModelIndex &parent = QModelIndex()) const ;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        QModelIndex index ( int row, int column = 0, const QModelIndex & parent = QModelIndex() ) const;

        KoAbstractResourceServerAdapter* getResourceAdapter(KoResource *resource) const;
        QModelIndex indexFromResource(KoResource* resource) const;
        QStringList assignedTagsList(KoResource *resource) const;
        bool removeResource(KoResource* resource);
        void addTag(KoResource* resource, const QString& tag);
        void deleteTag(KoResource *resource, const QString &tag);
        int resourcesCount() const;
        QList<KoResource *> currentlyVisibleResources() const;
        void updateServer();
        void enableResourceFiltering(bool enable);
        void searchTextChanged(const QString& searchString);
        void clearSelected();
        QList<QString> getSelectedResource();
        KoResource* getResourceFromFilename(const QString& filename);
        KoResource* getResourceFromIndex(const QModelIndex &index);

        //TODO A Suivre
        void refresh(){
            reset();
        }

        QStringList tagNamesList() const;
        void setCurrentTag(const QString& currentTag);
        QList<KoResource *> serverResources() const;
        void tagCategoryMembersChanged();
        void tagCategoryAdded(const QString& tag);
        void tagCategoryRemoved(const QString& tag);

    signals:
        void tagBoxEntryModified();
        void tagBoxEntryAdded(const QString& tag);
        void tagBoxEntryRemoved(const QString& tag);

    private slots:
        void tagBoxEntryWasModified();
        void tagBoxEntryWasAdded(const QString& tag);
        void tagBoxEntryWasRemoved(const QString& tag);
        void resourceAdded(KoResource *resource);
        void resourceRemoved(KoResource *resource);
        void resourceChanged(KoResource* resource);

        void resourceSelected(QModelIndex targetIndex);
        void allSelected(int index);


    private:
        QList<QString> m_resourceSelected;
        QList<KoAbstractResourceServerAdapter*> m_resourceAdapterList;
        QList<KoResource*> m_resources;
};

#endif // KORESOURCETABLEMODEL_H

