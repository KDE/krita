/*  This file is part of the KDE project

    Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>

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
    virtual void importResourceFile( const QString & filename ) = 0;
    virtual QString extensions() = 0;

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

    QList<KoResource*> resources() 
    {
        if( ! m_resourceServer )
            return QList<KoResource*>();

        QList<T*> serverResources = m_resourceServer->resources();

        QList<KoResource*> resources;
        foreach( T* resource, serverResources ) {
            resources.append( resource );
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

    void importResourceFile( const QString & filename )
    {
        if( ! m_resourceServer )
            return;
        m_resourceServer->importResourceFile(filename);
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

private:
    KoResourceServer<T>* m_resourceServer;
};

#endif // KO_RESOURCESERVER_ADAPTER_H_
