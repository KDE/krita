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
#include "KoResourceBundle.h"
#include <QAbstractItemModel>
#include <klocale.h>

#include <iostream>
using namespace std;

KoResourceTableModel::KoResourceTableModel(QList<QSharedPointer<KoAbstractResourceServerAdapter> > resourceAdapterList, int t, QObject *parent)
    : KoResourceModelBase( parent ), m_resourceAdapterList(resourceAdapterList), m_dataType(t)
{
    m_resourceSelected.clear();

    for (int i=0;i<m_resourceAdapterList.size();i++) {
        QSharedPointer<KoAbstractResourceServerAdapter> resourceAdapter=m_resourceAdapterList.at(i);
        Q_ASSERT( resourceAdapter );
        resourceAdapter->connectToResourceServer();

        //Bundle server must be at the end of the list
        if (i==m_resourceAdapterList.size()-1 && t!=Undefined) {
            refreshBundles(true);
        }
        else {
            m_resources.append(resourceAdapter->resources());
        }

        connect(resourceAdapter.data(), SIGNAL(resourceAdded(KoResource*)),
                this, SLOT(resourceAdded(KoResource*)));
        connect(resourceAdapter.data(), SIGNAL(removingResource(KoResource*)),
                this, SLOT(resourceRemoved(KoResource*)));
        connect(resourceAdapter.data(), SIGNAL(resourceChanged(KoResource*)),
                this, SLOT(resourceChanged(KoResource*)));
        connect(resourceAdapter.data(), SIGNAL(tagsWereChanged()),
                this, SLOT(tagBoxEntryWasModified()));
        connect(resourceAdapter.data(), SIGNAL(tagCategoryWasAdded(QString)),
                this, SLOT(tagBoxEntryWasAdded(QString)));
        connect(resourceAdapter.data(), SIGNAL(tagCategoryWasRemoved(QString)),
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
        else if (role == Qt::EditRole) {
            if (index.column()==1) {
                QString newVal=resource->name().section('.',0,0);
                return i18n( newVal.toUtf8().data());
            }
            else {
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

Qt::ItemFlags KoResourceTableModel::flags ( const QModelIndex & index ) const
{
    Qt::ItemFlags res=QAbstractItemModel::flags(index);
    if (index.column()==1) {
        res |= Qt::ItemIsEditable;
    }
    return res;
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

QSharedPointer<KoAbstractResourceServerAdapter> KoResourceTableModel::getResourceAdapter
    (KoResource *resource) const
{
    QSharedPointer<KoAbstractResourceServerAdapter> res;

    if (resource!=0) {
        for (int i=0;i<m_resourceAdapterList.size();i++) {
            res=m_resourceAdapterList.at(i);
            if (res->resources().indexOf(resource)!=-1) {
                return res;
            }
        }
    }
    return res;
}

QSharedPointer<KoAbstractResourceServerAdapter> KoResourceTableModel::getResourceAdapter
    (QString resourceName)
{
    return getResourceAdapter(getResourceFromFilename(resourceName));
}

QList<QSharedPointer<KoAbstractResourceServerAdapter> > KoResourceTableModel::getSelectedAdapters()
{
    QList<QSharedPointer<KoAbstractResourceServerAdapter> > res;
    QSharedPointer<KoAbstractResourceServerAdapter> currentAdapter;

    if (newSelection) {
        for (int i=0;i<m_resourceSelected.size();i++) {
            currentAdapter=getResourceAdapter(m_resourceSelected.at(i));
            if (!res.contains(currentAdapter)){
                res.append(currentAdapter);
            }
        }
        m_selectedAdapterList=res;
        newSelection=false;
    }

    return m_selectedAdapterList;
}

//TODO Vérifier s'il faut persister la sélection entre les modèles
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
        newSelection=true;
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

//TODO Rajouter le traitement de la liste de sélection si besoin
void KoResourceTableModel::refreshResources()
{
    m_resources.clear();

    for (int i=0;i<m_resourceAdapterList.size()-1;i++) {
        m_resources.append(m_resourceAdapterList.at(i)->resources());
    }

    refreshBundles(true);
}

void KoResourceTableModel::refreshBundles(bool isResourcesEmpty)
{
    KoResource* currentResource;
    KoResourceBundle* currentBundle;

    if (m_dataType!=Undefined) {
        QList<KoResource*> resourcesList=m_resourceAdapterList.at(m_resourceAdapterList.size()-1)->resources();
        for (int j=0;j<resourcesList.size();j++) {
            currentResource=resourcesList.at(j);
            currentBundle=dynamic_cast<KoResourceBundle*>(currentResource);
            if (currentBundle &&
                    ((m_dataType==Installed && currentBundle->isInstalled()) ||
                    (m_dataType==Available && !currentBundle->isInstalled())) &&
                        (isResourcesEmpty || !m_resources.contains(currentResource))) {
                m_resources.append(resourcesList.at(j));
            }
        }
        reset();
    }
}

void KoResourceTableModel::removeResourceFile(KoResource* resource,const QString &filename)
{
    removeOneSelected(filename);
    getResourceAdapter(resource)->removeResourceFile(filename);
}

void KoResourceTableModel::hideResource(KoResource* resource)
{
    removeOneSelected(resource->filename());
    m_resources.removeOne(resource);
}

void KoResourceTableModel::removeOneSelected(const QString &filename)
{
    if (!m_resourceSelected.isEmpty()) {
        m_resourceSelected.removeOne(filename);
    }
}

int KoResourceTableModel::getDataType()
{
    return m_dataType;
}

/*A tester : Implémentation des fonctions virtuelles de KoResourceModelBase*/

QModelIndex KoResourceTableModel::indexFromResource(KoResource* resource) const
{
    return index(m_resources.indexOf(resource), 1);
}

//TODO Supprimer le cout une fois tous les pbs réglés.
QStringList KoResourceTableModel::assignedTagsList(KoResource *resource) const
{
    if(!getResourceAdapter(resource))
        cout<<qPrintable(resource->filename())<<endl;
    return getResourceAdapter(resource)->assignedTagsList(resource);
}

bool KoResourceTableModel::removeResource(KoResource* resource)
{
    m_resources.removeOne(resource);
    m_resourceSelected.removeOne(resource->filename());
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

void KoResourceTableModel::tagCategoryAdded(const QString& tag)
{
    QList<QSharedPointer<KoAbstractResourceServerAdapter> > currentAdapters=getSelectedAdapters();

    for (int i=0;i<currentAdapters.size();i++) {
        currentAdapters.at(i)->tagCategoryAdded(tag);
    }
}

void KoResourceTableModel::tagCategoryRemoved(const QString& tag)
{
    QList<QSharedPointer<KoAbstractResourceServerAdapter> > currentAdapters=getSelectedAdapters();
    for (int i=0;i<currentAdapters.size();i++) {
        currentAdapters.at(i)->tagCategoryRemoved(tag);
    }
}

void KoResourceTableModel::tagCategoryMembersChanged()
{
    QList<QSharedPointer<KoAbstractResourceServerAdapter> > currentAdapters=getSelectedAdapters();
    for (int i=0;i<currentAdapters.size();i++) {
        currentAdapters.at(i)->tagCategoryMembersChanged();
    }
}

void KoResourceTableModel::setCurrentTag(const QString& currentTag)
{
    QList<QSharedPointer<KoAbstractResourceServerAdapter> > currentAdapters=getSelectedAdapters();
    for (int i=0;i<currentAdapters.size();i++) {
        currentAdapters.at(i)->setCurrentTag(currentTag);
    }
}

void KoResourceTableModel::updateServer()
{
    for (int i=0;i<m_resourceAdapterList.size();i++) {
        m_resourceAdapterList.at(i)->updateServer();
    }
    refreshResources();
}

void KoResourceTableModel::enableResourceFiltering(bool enable)
{
    for (int i=0;i<m_resourceAdapterList.size();i++) {
        m_resourceAdapterList.at(i)->enableResourceFiltering(enable);
    }
}

//TODO Vérifier que le refreshResources est indispensable
//TODO Bug au niveau des lettres qui ne sont pas contenues dans les deux ressources de type gradient
void KoResourceTableModel::searchTextChanged(const QString& searchString)
{    
    if(!searchString.isEmpty()){
        for (int i=0;i<m_resourceAdapterList.size();i++) {
            m_resourceAdapterList.at(i)->searchTextChanged(searchString);
        }
        refreshResources();
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

QStringList KoResourceTableModel::tagNamesList() const
{
    QStringList res=m_resourceAdapterList.at(0)->tagNamesList();
    for (int i=0;i<m_resourceAdapterList.size();i++) {
        res.append(m_resourceAdapterList.at(i)->tagNamesList());
    }
    res.removeDuplicates();
    return res;
}

QList< KoResource* > KoResourceTableModel::serverResources() const
{
    QList< KoResource* > res=m_resourceAdapterList.at(0)->serverResources();
    for (int i=0;i<m_resourceAdapterList.size();i++) {
        res.append(m_resourceAdapterList.at(i)->serverResources());
    }
    return res;
}

//TODO Voir si on peut optimiser
void KoResourceTableModel::resourceAdded(KoResource *resource)
{
    Q_UNUSED(resource);
    refreshResources();
}

void KoResourceTableModel::resourceRemoved(KoResource *resource)
{
    if (resource) {
        m_resourceSelected.removeOne(resource->filename());
        m_resources.removeOne(resource);
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

/*void KoResourceModel::importResourceFile(const QString &filename)
{
    m_resourceAdapter->importResourceFile(filename);
}

void KoResourceModel::importResourceFile(const QString & filename, bool fileCreation)
{
    m_resourceAdapter->importResourceFile(filename, fileCreation);
}

QStringList KoResourceModel::searchTag(const QString& lineEditText)
{
    return m_resourceAdapter->searchTag(lineEditText);
}

QString KoResourceTableModel::extensions() const
{
    return m_resourceAdapter->extensions();
}
*/

