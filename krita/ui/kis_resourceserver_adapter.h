/*
 *  Copyright (c) 2007 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_RESOURCESERVER_ADAPTER_H_
#define KIS_RESOURCESERVER_ADAPTER_H_

#include <KoResourceServer.h>
#include <KoResourceItemChooser.h>

#include <krita_export.h>

/// The resource server adapter provides a adapter pattern for a templated resource server
class KisAbstractResourceServerAdapter : public QObject
{
    Q_OBJECT
public:
    KisAbstractResourceServerAdapter();
    virtual ~KisAbstractResourceServerAdapter();

     virtual QList<KoResource*> resources() = 0;
     virtual bool addResource(KoResource* resource) = 0;
     virtual bool removeResource(KoResource* resource) = 0;
     virtual KoResource* importResource( const QString & filename ) = 0;

signals:
    void resourceAdded(KoResource*);
    void removingResource(KoResource*);

protected:
    void emitResourceAdded(KoResource* resource);
    void emitRemovingResource(KoResource* resource);
};

template <class T> class KisResourceServerAdapter : public KisAbstractResourceServerAdapter, public KoResourceServerObserver<T>
{
public:
    KisResourceServerAdapter(KoResourceServer<T>* resourceServer)
        : KisAbstractResourceServerAdapter()
        , m_resourceServer(resourceServer)
    {
        m_resourceServer->addObserver(this);
    }

    virtual ~KisResourceServerAdapter()
    {
        m_resourceServer->removeObserver(this);
    }

    QList<KoResource*> resources() {
        QList<T*> serverResources = m_resourceServer->resources();

        QList<KoResource*> resources;
        foreach( T* resource, serverResources ) {
            resources.append( resource );
        }
        return resources;
    }

    bool addResource(KoResource* resource)
    {
        T* res = dynamic_cast<T*>(resource);
        if(res)
            return m_resourceServer->addResource(res);

        return false;
    }

    bool removeResource(KoResource* resource)
    {
        T* res = dynamic_cast<T*>(resource);
        if(res)
            return m_resourceServer->removeResource(res);

        return false;
    }

    KoResource* importResource( const QString & filename ) {
        return m_resourceServer->importResource(filename);
    }

    void resourceAdded(T* resource)
    {
        emitResourceAdded(resource);
    }

    void removingResource(T* resource)
    {
        emitRemovingResource(resource);
    }

private:
    KoResourceServer<T>* m_resourceServer;
};

#endif // KIS_RESOURCESERVER_ADAPTER_H_
