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

#include "KoResourceTableModel.h"
#include <klocale.h>

#include <iostream>
using namespace std;

KoResourceTableModel::KoResourceTableModel(QList<KoAbstractResourceServerAdapter*> resourceAdapterList,QObject *parent)
    : KoResourceModelBase( parent ), m_resourceAdapterList(resourceAdapterList)
{
    m_resources.clear();
    m_resourceSelected.clear();

    for (int i=0;i<m_resourceAdapterList.size();i++) {
        KoAbstractResourceServerAdapter* resourceAdapter=m_resourceAdapterList.at(i);
        Q_ASSERT( resourceAdapter );
        resourceAdapter->connectToResourceServer();

        m_resources.append(resourceAdapter->resources());

        connect(resourceAdapter, SIGNAL(resourceAdded(KoResource*)),
                this, SLOT(resourceAdded(KoResource*)));
        connect(resourceAdapter, SIGNAL(removingResource(KoResource*)),
                this, SLOT(resourceRemoved(KoResource*)));
        connect(resourceAdapter, SIGNAL(resourceChanged(KoResource*)),
                this, SLOT(resourceChanged(KoResource*)));
        connect(resourceAdapter, SIGNAL(tagsWereChanged()),
                this, SLOT(tagBoxEntryWasModified()));
        connect(resourceAdapter, SIGNAL(tagCategoryWasAdded(QString)),
                this, SLOT(tagBoxEntryWasAdded(QString)));
        connect(resourceAdapter, SIGNAL(tagCategoryWasRemoved(QString)),
                this, SLOT(tagBoxEntryWasRemoved(QString)));
    }
}

int KoResourceTableModel::rowCount(const QModelIndex & /*parent*/) const
{
    return resourcesCount();
}

int KoResourceTableModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 3;
}

QVariant KoResourceTableModel::data(const QModelIndex &index, int role) const
{
    KoResource * resource = static_cast<KoResource*>(index.internalPointer());
    if (!resource) {
        return QVariant();
    }
    else {
        if (role == Qt::DecorationRole && index.column() == 1) {
            if (resource->image().isNull()) {
                return QVariant();
            }
            else {
                return QVariant(resource->image().scaledToWidth(50));
            }
        }
        else if (role == Qt::DisplayRole)
        {
            switch (index.column()) {
            case 1:
                return i18n( resource->name().toUtf8().data());
            case 2:
                if (assignedTagsList(resource).count()) {
                    QString taglist = assignedTagsList(resource).join("] , [");
                    return QString("[%1]").arg(taglist);
                }
                else {
                    return QVariant();
                }
            default:
                return QVariant();
            }
        }
        else if (role == Qt::CheckStateRole && index.column()==0) {
            return m_resourceSelected.contains(resource->filename()) ? Qt::Checked : Qt::Unchecked;
        }
        else {
            return QVariant();
        }
    }
}

QVariant KoResourceTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString();
            case 1:
                return QString("Name");
            case 2:
                return QString("Tags");
            }
        }
    }
    return QVariant();
}

QModelIndex KoResourceTableModel::index ( int row, int column, const QModelIndex & ) const
{
    const QList<KoResource*> resources = currentlyVisibleResources();
    if ( row >= resources.count() || row < 0) {
        return QModelIndex();
    }
    else {
        return createIndex( row, column, resources[row] );
    }

}

KoAbstractResourceServerAdapter* KoResourceTableModel::getResourceAdapter
    (KoResource *resource) const
{
    KoAbstractResourceServerAdapter* res;

    for (int i=0;i<m_resourceAdapterList.size();i++) {
        res=m_resourceAdapterList.at(i);
        if (res->resources().indexOf(resource)!=-1) {
            return res;
        }
    }
    return 0;
}

QModelIndex KoResourceTableModel::indexFromResource(KoResource* resource) const
{
    return index(m_resources.indexOf(resource), 1);
}

QStringList KoResourceTableModel::assignedTagsList(KoResource *resource) const
{
    return getResourceAdapter(resource)->assignedTagsList(resource);
}

bool KoResourceTableModel::removeResource(KoResource* resource)
{
    return getResourceAdapter(resource)->removeResource(resource);
}

void KoResourceTableModel::addTag(KoResource* resource, const QString& tag)
{
    getResourceAdapter(resource)->addTag(resource, tag);
    emit tagBoxEntryAdded(tag);
}

void KoResourceTableModel::deleteTag(KoResource *resource, const QString &tag)
{
    getResourceAdapter(resource)->deleteTag(resource, tag);
}

int KoResourceTableModel::resourcesCount() const
{
    return m_resources.count();
}

QList<KoResource *> KoResourceTableModel::currentlyVisibleResources() const
{
  return m_resources;
}

//TODO Vérifier la validité de toutes les méthodes ci-dessous


void KoResourceTableModel::resourceAdded(KoResource *resource)
{
    m_resources.append(resource);
    reset();
}

void KoResourceTableModel::resourceRemoved(KoResource *resource)
{
    int index=m_resources.indexOf(resource);
    if (index!=-1) {
        m_resources.removeAt(index);
    }
    reset();
}

void KoResourceTableModel::resourceChanged(KoResource* resource)
{
    QModelIndex modelIndex = index(m_resources.indexOf(resource),1);
    if (!modelIndex.isValid()) {
        return;
    }

    emit dataChanged(modelIndex, modelIndex);
}

void KoResourceTableModel::resourceSelected(QModelIndex targetIndex)
{
    if (targetIndex.column()==0) {
        QString filename = currentlyVisibleResources().at(targetIndex.row())->filename();

        if (m_resourceSelected.contains(filename)) {
            m_resourceSelected.removeOne(filename);
        }
        else {
            m_resourceSelected.append(filename);
        }
        reset();
    }
}

void KoResourceTableModel::tagBoxEntryWasModified()
{
    updateServer();
    emit tagBoxEntryModified();
}

void KoResourceTableModel::tagBoxEntryWasAdded(const QString& tag)
{
    emit tagBoxEntryAdded(tag);
}

void KoResourceTableModel::tagBoxEntryWasRemoved(const QString& tag)
{
    emit tagBoxEntryRemoved(tag);
}

void KoResourceTableModel::updateServer()
{
    for (int i=0;i<m_resourceAdapterList.size();i++) {
        m_resourceAdapterList.at(i)->updateServer();
    }
}

void KoResourceTableModel::enableResourceFiltering(bool enable)
{
    for (int i=0;i<m_resourceAdapterList.size();i++) {
        m_resourceAdapterList.at(i)->enableResourceFiltering(enable);
    }
}

void KoResourceTableModel::searchTextChanged(const QString& searchString)
{
    for (int i=0;i<m_resourceAdapterList.size();i++) {
        m_resourceAdapterList.at(i)->searchTextChanged(searchString);
    }
}

void KoResourceTableModel::allSelected(int index)
{
    if (index==0) {
        if (m_resourceSelected.size()<m_resources.size()) {
            m_resourceSelected.clear();
            for (int i=0;i<m_resources.size();i++) {
                m_resourceSelected.append(m_resources.at(i)->filename());
            }
        }
        else {
            m_resourceSelected.clear();
        }
    }
    reset();
}

void KoResourceTableModel::clearSelected()
{
    m_resourceSelected.clear();
}

QList<QString> KoResourceTableModel::getSelectedResource()
{
    return m_resourceSelected;
}

QStringList KoResourceTableModel::tagNamesList() const
{
    QStringList res=m_resourceAdapterList.at(0)->tagNamesList();
    for (int i=0;i<m_resourceAdapterList.size();i++) {
        res.append(m_resourceAdapterList.at(i)->tagNamesList());
    }
    return res;
}

void KoResourceTableModel::setCurrentTag(const QString& currentTag)
{
    Q_UNUSED(currentTag);
}

QList< KoResource* > KoResourceTableModel::serverResources() const
{
    QList< KoResource* > res=m_resourceAdapterList.at(0)->serverResources();
    for (int i=0;i<m_resourceAdapterList.size();i++) {
        res.append(m_resourceAdapterList.at(i)->serverResources());
    }
    return res;
}

void KoResourceTableModel::tagCategoryMembersChanged()
{

}

void KoResourceTableModel::tagCategoryAdded(const QString& tag)
{
    Q_UNUSED(tag);
}

void KoResourceTableModel::tagCategoryRemoved(const QString& tag)
{
    Q_UNUSED(tag);
}

KoResource* KoResourceTableModel::getResourceFromFilename(const QString& filename)
{
    KoResource* res;

    for (int i=0;i<m_resources.size();i++) {
        res=m_resources.at(i);
        if (res->filename()==filename) {
            return res;
        }
    }

    return 0;
}

KoResource* KoResourceTableModel::getResourceFromIndex(const QModelIndex &index)
{
    KoResource * resource = static_cast<KoResource*>(index.internalPointer());
    if (!resource) {
        return 0;
    }
    else {
        return resource;
    }
}

//TODO Penser à un syst d'identification du serveur concerné
//Pour les fonctions suivantes si nécessaires
//(Exemple : string en paramètre, conversion vers indice dans la liste des serveurs...)

/*void KoResourceModel::importResourceFile(const QString &filename)
{
    m_resourceAdapter->importResourceFile(filename);
}

void KoResourceModel::importResourceFile(const QString & filename, bool fileCreation)
{
    m_resourceAdapter->importResourceFile(filename, fileCreation);
}

void KoResourceModel::removeResourceFile(const QString &filename)
{
    m_resourceAdapter->removeResourceFile(filename);
}

QStringList KoResourceModel::searchTag(const QString& lineEditText)
{
    return m_resourceAdapter->searchTag(lineEditText);
}

void KoResourceModel::searchTextChanged(const QString& searchString)
{
    m_resourceAdapter->searchTextChanged(searchString);
}

void KoResourceModel::enableResourceFiltering(bool enable)
{
    m_resourceAdapter->enableResourceFiltering(enable);
}

QString KoResourceTableModel::extensions() const
{
    return m_resourceAdapter->extensions();
}
*/

