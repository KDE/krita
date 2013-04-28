/*  This file is part of the KDE project

    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>

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
    virtual bool addResource(KoResource* resource) = 0;
    virtual bool removeResource(KoResource* resource) = 0;
    virtual void removeResourceFile(const QString & filename) = 0;
    virtual void importResourceFile(const QString & filename, bool fileCreation=true) = 0;
    virtual QString extensions() = 0;
    virtual void setTaggedResourceFileNames(const QStringList& resourceFileNames)=0;
    virtual void setTagSearch(bool tagSearch)=0;
    virtual void updateServer()=0;
    virtual QStringList getAssignedTagsList( KoResource* resource )=0;
    virtual QStringList getTagNamesList()=0;
    virtual void addTag( KoResource* resource,const QString& tag)=0;
    virtual void deleteTag( KoResource* resource,const QString& tag)=0;
    virtual QStringList searchTag(const QString& lineEditText)=0;

signals:
    void resourceAdded(KoResource*);
    void removingResource(KoResource*);
    void resourceChanged(KoResource*);

protected:
    void emitResourceAdded(KoResource* resource);
    void emitRemovingResource(KoResource* resource);
    void emitResourceChanged(KoResource* resource);
};

/**
 * The KoResourceServerAdapter provides adapter to a specific resource server
 * It provides a resource type independent interface to the server.
 */
template <class T> class KoResourceServerAdapter : public KoAbstractResourceServerAdapter, public KoResourceServerObserver<T>
{
public:
    KoResourceServerAdapter(KoResourceServer<T>* resourceServer, QObject *parent = 0)
        : KoAbstractResourceServerAdapter(parent)
        , m_resourceServer(resourceServer)
    {
        m_tagSearch=false;
    }

    virtual ~KoResourceServerAdapter()
    {
        if( m_resourceServer )
            m_resourceServer->removeObserver(this);
    }

    void connectToResourceServer()
    {
        if( m_resourceServer )
            m_resourceServer->addObserver(this);
    }

    virtual QList<KoResource*> resources() 
    {
        if( ! m_resourceServer )
            return QList<KoResource*>();

        QList<T*> serverResources = m_resourceServer->resources();

        QList<KoResource*> resources;

        foreach( T* resource, serverResources ) {
            resources.append( resource );
        }

        if(m_tagSearch) {
            foreach(KoResource* resource, resources) {
                if(!m_resourceFileNames.contains(resource->filename())) {
                    resources.removeAll(resource);
                }
            }
        }

        return resources;
    }

    bool addResource(KoResource* resource)
    {
        if( ! m_resourceServer )
            return false;

        T* res = dynamic_cast<T*>(resource);
        if(res)
            return m_resourceServer->addResource(res);

        return false;
    }

    bool removeResource(KoResource* resource)
    {
        if( ! m_resourceServer )
            return false;

        T* res = dynamic_cast<T*>(resource);
        if(res)
            return m_resourceServer->removeResource(res);

        return false;
    }

    void importResourceFile(const QString & filename , bool fileCreation=true)
    {
        if( ! m_resourceServer )
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

    void resourceAdded(T* resource)
    {
        emitResourceAdded(resource);
    }

    void removingResource(T* resource)
    {
        emitRemovingResource(resource);
    }
    
    void resourceChanged(T* resource)
    {
        emitResourceChanged(resource);
    }

    QString extensions()
    {
        if( ! m_resourceServer )
            return QString();

        return m_resourceServer->extensions();
    }
    
    void setTaggedResourceFileNames(const QStringList& resourceFileNames)
    {
        m_resourceFileNames = resourceFileNames;
    }

    void setTagSearch(bool tagSearch )
    {
        m_tagSearch = tagSearch;
    }

    void updateServer()
    {
        emitRemovingResource(0);
    }

    QStringList getAssignedTagsList( KoResource* resource )
    {
        return m_resourceServer->getAssignedTagsList(resource);
    }

    QStringList getTagNamesList()
    {
        return m_resourceServer->getTagNamesList();
    }

    void addTag( KoResource* resource,const QString& tag)
    {
        m_resourceServer->addTag(resource,tag);
    }

    void deleteTag( KoResource* resource,const QString& tag)
    {
        m_resourceServer->delTag(resource,tag);
    }

    QStringList searchTag(const QString& lineEditText)
    {
        return m_resourceServer->searchTag(lineEditText);
    }

protected:
    KoResourceServer<T>* resourceServer()
    {
        return m_resourceServer;
    }

private:
    KoResourceServer<T>* m_resourceServer;
    QStringList m_resourceFileNames;
    bool m_tagSearch;
};

#endif // KO_RESOURCESERVER_ADAPTER_H_
