/*  This file is part of the KDE project

    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

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

#include "kritawidgets_export.h"

/// The resource server adapter provides a adapter pattern for a templated resource server
class KRITAWIDGETS_EXPORT KoAbstractResourceServerAdapter : public QObject
{
    Q_OBJECT
public:
    KoAbstractResourceServerAdapter(QObject *parent = 0);
    ~KoAbstractResourceServerAdapter() override;

    virtual void connectToResourceServer() = 0;
    virtual QList<KoResourceSP> resources() = 0;

    virtual QList<KoResourceSP> serverResources() = 0;

    virtual bool addResource(KoResourceSP resource) = 0;
    virtual bool removeResource(KoResourceSP resource) = 0;
    virtual void removeResourceFile(const QString & filename) = 0;
    virtual void importResourceFile(const QString & filename, bool fileCreation = true) = 0;
    virtual QString extensions() const = 0;
    virtual void setCurrentTag(const QString& currentTag) = 0;
    virtual void enableResourceFiltering(bool tagSearch) = 0;
    virtual void updateServer() = 0;
    virtual QStringList assignedTagsList(KoResourceSP resource) = 0;
    virtual QStringList tagNamesList() = 0;
    virtual void addTag(const QString& tag) = 0;
    virtual void addTag(KoResourceSP resource, const QString& tag) = 0;
    virtual void deleteTag(KoResourceSP resource, const QString& tag) = 0;
    virtual void searchTextChanged(const QString& searchString) = 0;
    // these call the server.
    virtual void tagCategoryMembersChanged() = 0;
    virtual void tagCategoryAdded(const QString& tag) = 0;
    virtual void tagCategoryRemoved(const QString& tag) = 0;

    virtual void setFilterIncludes(const QStringList& filteredNames) = 0;
    virtual QStringList searchTag(const QString& lineEditText) = 0;
    virtual void configureFilters(int filterType, bool enable) = 0;

    virtual QString serverType() const { return QString(); }

    virtual void setSortingEnabled(bool value) = 0;
    virtual bool sortingEnabled() const = 0;

Q_SIGNALS:
    void resourceAdded(KoResourceSP );
    void removingResource(KoResourceSP );
    void resourceChanged(KoResourceSP );
    void tagsWereChanged();
    void tagCategoryWasAdded(const QString& tag);
    void tagCategoryWasRemoved(const QString& tag);

protected:
    void emitResourceAdded(KoResourceSP resource);
    void emitRemovingResource(KoResourceSP resource);
    void emitResourceChanged(KoResourceSP resource);
    void emitTagsWereChanged();
    void emitTagCategoryWasAdded(const QString& tag);
    void emitTagCategoryWasRemoved(const QString& tag);
};

/**
 * The KoResourceServerAdapter provides adapter to a specific resource server
 * It provides a resource type independent interface to the server.
 */
template <class T>
    class KoResourceServerAdapter : public KoAbstractResourceServerAdapter, public KoResourceServerObserver<T>
{
    typedef KoResourceServer<T> ServerType;
public:
    KoResourceServerAdapter(ServerType* resourceServer, QObject *parent = 0)
        : KoAbstractResourceServerAdapter(parent)
        , m_resourceServer(resourceServer)
        , m_sortingEnabled(false)
    {
        m_changeCounter = 0;
        m_oldChangeCounter = 0;
        m_enableFiltering = false;
        m_resourceFilter.setResourceServer(m_resourceServer);
    }


    ~KoResourceServerAdapter() override
    {
        if (m_resourceServer)
            m_resourceServer->removeObserver(this);
    }

    QString serverType() const override
    {
        if (m_resourceServer) {
            return m_resourceServer->type();
        }
        return KoAbstractResourceServerAdapter::serverType();
    }

    void unsetResourceServer() override
    {
        m_resourceServer = 0;
    }

    void connectToResourceServer() override
    {
        if (m_resourceServer)
            m_resourceServer->addObserver(this);
    }

    QList<KoResourceSP > resources() override
    {
        if (! m_resourceServer)
            return QList<KoResourceSP >();

        bool cacheDirty = serverResourceCacheInvalid();
        if (cacheDirty) {
            QList<QSharedPointer<T>> serverResources =
                m_sortingEnabled ?
                m_resourceServer->sortedResources() :
                m_resourceServer->resources();

            m_serverResources.clear();

            Q_FOREACH (QSharedPointer<T> resource, serverResources) {
                m_serverResources.append(resource);
            }
            serverResourceCacheInvalid(false);

        }
        if (m_enableFiltering) {
            if (m_resourceFilter.filtersHaveChanged() || cacheDirty) {
                m_filteredResources = m_resourceFilter.filterResources(m_serverResources);
            }
            return m_filteredResources;
        }
        return m_serverResources;
    }

    bool addResource(KoResourceSP resource) override
    {
        if (! m_resourceServer)
            return false;

        QSharedPointer<T> res = resource.dynamicCast<T>();
        if (res) {
            return m_resourceServer->addResource(res);
        }

        return false;
    }

    bool removeResource(KoResourceSP resource) override
    {
        if (! m_resourceServer)
            return false;

        QSharedPointer<T> res = resource.dynamicCast<T>();
        if (res) {
            return m_resourceServer->removeResourceAndBlacklist(res);
        }

        return false;
    }

    void importResourceFile(const QString & filename , bool fileCreation = true) override
    {
        if (! m_resourceServer)
            return;
        m_resourceServer->importResourceFile(filename, fileCreation);
    }

    void removeResourceFile(const QString & filename) override
    {
        if (!m_resourceServer) {
            return;
        }

        m_resourceServer->removeResourceFile(filename);
    }

    void resourceAdded(QSharedPointer<T> resource) override {
        serverResourceCacheInvalid(true);
        emitResourceAdded(resource);
    }

    void removingResource(QSharedPointer<T> resource) override {
        serverResourceCacheInvalid(true);
        emitRemovingResource(resource);
    }

    void resourceChanged(QSharedPointer<T> resource) override {
        serverResourceCacheInvalid(true);
        emitResourceChanged(resource);
    }

    void resourceChangedNoCacheInvalidation(QSharedPointer<T> resource) {
        emitResourceChanged(resource);
    }

    void syncTaggedResourceView() override {
        serverResourceCacheInvalid(true);
        m_resourceFilter.rebuildCurrentTagFilenames();
        emitTagsWereChanged();
    }

    void syncTagAddition(const QString& tag) override {
        emitTagCategoryWasAdded(tag);
    }

    void syncTagRemoval(const QString& tag) override {
        emitTagCategoryWasRemoved(tag);
    }

    QString extensions() const override {
        if (! m_resourceServer)
            return QString();

        return m_resourceServer->extensions();
    }

    void setCurrentTag(const QString& resourceFileNames) override {
        serverResourceCacheInvalid(true);
        m_resourceFilter.setCurrentTag(resourceFileNames);
    }

    void enableResourceFiltering(bool enable) override {
        m_enableFiltering = enable;
    }

    void updateServer() override {
        emitRemovingResource(0);
    }

    QStringList assignedTagsList(KoResourceSP resource) override {
        return m_resourceServer->assignedTagsList(resource);
    }

    QStringList tagNamesList() override {
        return m_resourceServer->tagNamesList();
    }

    void addTag(const QString& tag) override {
        m_resourceServer->addTag(0, tag);
    }

    void addTag(KoResourceSP resource, const QString& tag) override {
        m_resourceServer->addTag(resource, tag);
    }

    void deleteTag(KoResourceSP resource, const QString& tag) override {
        m_resourceServer->delTag(resource, tag);
    }

    void setFilterIncludes(const QStringList& filteredNames) override {
        m_resourceFilter.setInclusions(filteredNames);
    }

    void searchTextChanged(const QString& searchString) override {
        m_resourceFilter.setFilters(searchString);
        serverResourceCacheInvalid(true);
    }

    QStringList searchTag(const QString& lineEditText) override {
        return m_resourceServer->searchTag(lineEditText);
    }

    // called by model to notify server of change
    void tagCategoryMembersChanged() override {
        m_resourceServer->tagCategoryMembersChanged();
    }

    void tagCategoryAdded(const QString& tag) override {
        m_resourceServer->tagCategoryAdded(tag);
    }

    void tagCategoryRemoved(const QString& tag) override {
        m_resourceServer->tagCategoryRemoved(tag);
    }
    QList<KoResourceSP > serverResources() override {
        return m_serverResources;
    }

    void configureFilters(int filterType, bool enable) override{
        m_resourceFilter.configure(filterType,enable);
    }

    void setSortingEnabled(bool value) override {
        m_sortingEnabled = value;
        serverResourceCacheInvalid(true);
    }

    bool sortingEnabled() const override {
        return m_sortingEnabled;
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

    ServerType* m_resourceServer;
    unsigned int m_changeCounter;
    unsigned int m_oldChangeCounter;
    QList<KoResourceSP > m_serverResources;
    QList<KoResourceSP > m_filteredResources;
    bool m_enableFiltering;
    bool m_sortingEnabled;
};

#endif // KO_RESOURCESERVER_ADAPTER_H_
