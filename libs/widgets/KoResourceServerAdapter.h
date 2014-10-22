/*  This file is part of the KDE project

    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#ifndef KO_RESOURCESERVER_ADAPTER_H_
#define KO_RESOURCESERVER_ADAPTER_H_

#include "KoResourceServer.h"
#include <KoResource.h>
#include <KoResourceFiltering.h>

#include "kowidgets_export.h"

/// The resource server adapter provides a adapter pattern for a templated resource server
class KOWIDGETS_EXPORT KoAbstractResourceServerAdapter : public QObject
{
    Q_OBJECT
public:
    KoAbstractResourceServerAdapter(QObject *parent = 0);
    virtual ~KoAbstractResourceServerAdapter();

    virtual void connectToResourceServer() = 0;
    virtual QList<KoResource*> resources() = 0;
    virtual QList<KoResource*> serverResources() = 0;
    virtual bool addResource(KoResource* resource) = 0;
    virtual bool removeResource(KoResource* resource) = 0;
    virtual void removeResourceFile(const QString & filename) = 0;
    virtual void importResourceFile(const QString & filename, bool fileCreation = true) = 0;
    virtual QString extensions() const = 0;
    virtual void setCurrentTag(const QString& currentTag) = 0;
    virtual void enableResourceFiltering(bool tagSearch) = 0;
    virtual void updateServer() = 0;
    virtual QStringList assignedTagsList(KoResource* resource) = 0;
    virtual QStringList tagNamesList() = 0;
    virtual void addTag(const QString& tag) = 0;
    virtual void addTag(KoResource* resource, const QString& tag) = 0;
    virtual void deleteTag(KoResource* resource, const QString& tag) = 0;
    virtual void searchTextChanged(const QString& searchString) = 0;
    // these call the server.
    virtual void tagCategoryMembersChanged() = 0;
    virtual void tagCategoryAdded(const QString& tag) = 0;
    virtual void tagCategoryRemoved(const QString& tag) = 0;

    virtual void setFilterIncludes(const QStringList& filteredNames) = 0;
    virtual QStringList searchTag(const QString& lineEditText) = 0;
    virtual void configureFilters(int filterType, bool enable) = 0;

    virtual QString serverType() const { return QString(); }

signals:
    void resourceAdded(KoResource*);
    void removingResource(KoResource*);
    void resourceChanged(KoResource*);
    void tagsWereChanged();
    void tagCategoryWasAdded(const QString& tag);
    void tagCategoryWasRemoved(const QString& tag);

protected:
    void emitResourceAdded(KoResource* resource);
    void emitRemovingResource(KoResource* resource);
    void emitResourceChanged(KoResource* resource);
    void emitTagsWereChanged();
    void emitTagCategoryWasAdded(const QString& tag);
    void emitTagCategoryWasRemoved(const QString& tag);
};

/**
 * The KoResourceServerAdapter provides adapter to a specific resource server
 * It provides a resource type independent interface to the server.
 */
template <class T, class Policy = PointerStroragePolicy<T> >
    class KoResourceServerAdapter : public KoAbstractResourceServerAdapter, public KoResourceServerObserver<T, Policy>
{
    typedef KoResourceServer<T, Policy> ServerType;
    typedef typename Policy::PointerType PointerType;
public:
    KoResourceServerAdapter(ServerType* resourceServer, QObject *parent = 0)
        : KoAbstractResourceServerAdapter(parent)
        , m_resourceServer(resourceServer)
    {
        m_changeCounter = 0;
        m_oldChangeCounter = 0;
        m_enableFiltering = false;
        m_resourceFilter.setTagStore(m_resourceServer->tagObject());
    }


    virtual ~KoResourceServerAdapter()
    {
        if (m_resourceServer)
            m_resourceServer->removeObserver(this);
    }

    QString serverType() const
    {
        if (m_resourceServer) {
            return m_resourceServer->type();
        }
        return KoAbstractResourceServerAdapter::serverType();
    }

    virtual void unsetResourceServer()
    {
        m_resourceServer = 0;
    }

    void connectToResourceServer()
    {
        if (m_resourceServer)
            m_resourceServer->addObserver(this);
    }

    virtual QList<KoResource*> resources()
    {
        if (! m_resourceServer)
            return QList<KoResource*>();

        bool cacheDirty = serverResourceCacheInvalid();
        if (cacheDirty) {
            cacheServerResources(m_resourceServer->resources());
        }
        if (m_enableFiltering) {
            if (m_resourceFilter.filtersHaveChanged() || cacheDirty) {
                m_filteredResources = m_resourceFilter.filterResources(m_serverResources);
            }
            return m_filteredResources;
        }
        return m_serverResources;
    }

    bool addResource(KoResource* resource)
    {
        if (! m_resourceServer)
            return false;

        T* res = dynamic_cast<T*>(resource);
        if (res) {
            return m_resourceServer->addResource(res);
        }

        return false;
    }

    bool removeResource(KoResource* resource)
    {
        if (! m_resourceServer)
            return false;

        T* res = dynamic_cast<T*>(resource);
        if (res) {

            return m_resourceServer->removeResourceAndBlacklist(res);

        }

        return false;
    }

    void importResourceFile(const QString & filename , bool fileCreation = true)
    {
        if (! m_resourceServer)
            return;
        m_resourceServer->importResourceFile(filename, fileCreation);
    }

    void removeResourceFile(const QString & filename)
    {
        if (!m_resourceServer) {
            return;
        }

        m_resourceServer->removeResourceFile(filename);
    }

    void resourceAdded(PointerType resource) {
        serverResourceCacheInvalid(true);
        emitResourceAdded(Policy::toResourcePointer(resource));
    }

    void removingResource(PointerType resource) {
        serverResourceCacheInvalid(true);
        emitRemovingResource(Policy::toResourcePointer(resource));
    }

    void resourceChanged(PointerType resource) {
        serverResourceCacheInvalid(true);
        emitResourceChanged(Policy::toResourcePointer(resource));
    }

    void syncTaggedResourceView() {
        serverResourceCacheInvalid(true);
        m_resourceFilter.rebuildCurrentTagFilenames();
        emitTagsWereChanged();
    }

    void syncTagAddition(const QString& tag) {
        emitTagCategoryWasAdded(tag);
    }

    void syncTagRemoval(const QString& tag) {
        emitTagCategoryWasRemoved(tag);
    }

    QString extensions() const {
        if (! m_resourceServer)
            return QString();

        return m_resourceServer->extensions();
    }

    void setCurrentTag(const QString& resourceFileNames) {
        serverResourceCacheInvalid(true);
        m_resourceFilter.setCurrentTag(resourceFileNames);
    }

    void enableResourceFiltering(bool enable) {
        m_enableFiltering = enable;
    }

    void updateServer() {
        emitRemovingResource(0);
    }

    QStringList assignedTagsList(KoResource* resource) {
        return m_resourceServer->assignedTagsList(resource);
    }

    QStringList tagNamesList() {
        return m_resourceServer->tagNamesList();
    }

    void addTag(const QString& tag) {
        m_resourceServer->addTag(0, tag);
    }

    void addTag(KoResource* resource, const QString& tag) {
        m_resourceServer->addTag(resource, tag);
    }

    void deleteTag(KoResource* resource, const QString& tag) {
        m_resourceServer->delTag(resource, tag);
    }

    void setFilterIncludes(const QStringList& filteredNames) {
        m_resourceFilter.setInclusions(filteredNames);
    }

    void searchTextChanged(const QString& searchString) {
        m_resourceFilter.setFilters(searchString);
        serverResourceCacheInvalid(true);
    }

    QStringList searchTag(const QString& lineEditText) {
        return m_resourceServer->searchTag(lineEditText);
    }

    // called by model to notify server of change
    void tagCategoryMembersChanged() {
        m_resourceServer->tagCategoryMembersChanged();
    }

    void tagCategoryAdded(const QString& tag) {
        m_resourceServer->tagCategoryAdded(tag);
    }

    void tagCategoryRemoved(const QString& tag) {
        m_resourceServer->tagCategoryRemoved(tag);
    }

    virtual QList<KoResource*> serverResources() {
        return m_serverResources;
    }

    void configureFilters(int filterType, bool enable){
        m_resourceFilter.configure(filterType,enable);
    }

protected:
    ServerType* resourceServer() const {
        return m_resourceServer;
    }
protected:
    KoResourceFiltering m_resourceFilter;
private:
    bool serverResourceCacheInvalid() const {
        return m_changeCounter != m_oldChangeCounter;
    }

    void serverResourceCacheInvalid(bool yes) {
        if (yes) {
            ++m_changeCounter;
        } else {
            m_oldChangeCounter = m_changeCounter;
        }
    }

    void cacheServerResources(const QList<PointerType> &serverResources) {
        m_serverResources.clear();

        foreach(PointerType resource, serverResources) {
            m_serverResources.append(Policy::toResourcePointer(resource));
        }
        serverResourceCacheInvalid(false);
    }

    ServerType* m_resourceServer;
    unsigned int m_changeCounter;
    unsigned int m_oldChangeCounter;
    QList<KoResource*> m_serverResources;
    QList<KoResource*> m_filteredResources;
    bool m_enableFiltering;
};

#endif // KO_RESOURCESERVER_ADAPTER_H_
