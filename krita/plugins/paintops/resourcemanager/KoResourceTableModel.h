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
#include <QItemDelegate>
#include <QLineEdit>

#include <iostream>
using namespace std;

class KoResource;

class KoResourceTableDelegate : public QItemDelegate
{
    Q_OBJECT

signals:
    void renameEnded(QString) const;

public:
    KoResourceTableDelegate(QObject *parent = 0) : QItemDelegate(parent){}

    //TODO Essayer de changer la taille/position de la lineEdit
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
    {
        Q_UNUSED(option);
        Q_UNUSED(index);
        QLineEdit *editor=new QLineEdit(parent);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QLineEdit *currentEditor = static_cast<QLineEdit*>(editor);
        currentEditor->setText(index.model()->data(index, Qt::EditRole).toString());
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const
    {
        Q_UNUSED(model);
        Q_UNUSED(index);
        QLineEdit *currentEditor = static_cast<QLineEdit*>(editor);
        emit renameEnded(currentEditor->text());
    }
};


class KoResourceTableModel : public KoResourceModelBase
{
    Q_OBJECT

    public:

        enum {
            Undefined=-1,
            Available,
            Installed,
            Blacklist
        };

        KoResourceTableModel(QList<QSharedPointer<KoAbstractResourceServerAdapter> >resourceAdapterList, int type=Available, QObject * parent = 0 );
        int rowCount(const QModelIndex &parent = QModelIndex()) const ;
        int columnCount(const QModelIndex &parent = QModelIndex()) const;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
        QVariant headerData(int section, Qt::Orientation orientation, int role) const;
        QModelIndex index ( int row, int column = 0, const QModelIndex & parent = QModelIndex() ) const;
        Qt::ItemFlags flags ( const QModelIndex & index ) const;

        KoResource* getResourceFromFilename(const QString& filename);
        KoResource* getResourceFromIndex(const QModelIndex &index);
        QSharedPointer<KoAbstractResourceServerAdapter> getResourceAdapter(KoResource *resource) const;
        QSharedPointer<KoAbstractResourceServerAdapter> getResourceAdapter(QString resourceName);
        QList<QSharedPointer<KoAbstractResourceServerAdapter> > getSelectedAdapters();
        void clearSelected();
        QList<QString> getSelectedResource();
        void refreshResources();
        void refreshBundles(bool isResourcesEmpty=false);
        void removeResourceFile(KoResource* resource,const QString &filename);
        void hideResource(KoResource* resource);
        void removeOneSelected(const QString& filename);
        int getDataType();
        void configureFilters(int filterType, bool enable);

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
        void resourceSelected(QModelIndex targetIndex);
        void allSelected(int index);

        void tagBoxEntryWasModified();
        void tagBoxEntryWasAdded(const QString& tag);
        void tagBoxEntryWasRemoved(const QString& tag);
        void resourceAdded(KoResource *resource);
        void resourceRemoved(KoResource *resource);
        void resourceChanged(KoResource* resource);

    private:
        QList<QString> m_resourceSelected;
        QList<QSharedPointer<KoAbstractResourceServerAdapter> > m_resourceAdapterList;
        QList<KoResource*> m_resources;
        QList<QSharedPointer<KoAbstractResourceServerAdapter> > m_selectedAdapterList;
        bool m_newSelection;
        int m_dataType;
};

#endif // KORESOURCETABLEMODEL_H

