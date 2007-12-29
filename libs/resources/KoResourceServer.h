/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef KORESOURCESERVER_H
#define KORESOURCESERVER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QFileInfo>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include "KoResource.h"
#include "KoResourceServerObserver.h"

#include "koresource_export.h"
#include <QMutex>
#include <QMutexLocker>
#include <KDebug>

class KoResource;

class KORESOURCES_EXPORT KoResourceServerBase {

public:
    KoResourceServerBase(const QString& type): m_type(type) {}
    virtual ~KoResourceServerBase() {}

    virtual void loadResources(QStringList filenames) = 0;
    QString type() { return m_type; }

private:
    QString m_type;

protected:

    QMutex loadLock;

};

template <class T> class KoResourceServer : public KoResourceServerBase {

public:
    KoResourceServer(const QString& type)
        : KoResourceServerBase(type)
        {
        }
        
    virtual ~KoResourceServer()
        {
        }

    void loadResources(QStringList filenames) {
        kDebug(30009) << "loading  resources for type " << type();
        QStringList uniqueFiles;

        while (!filenames.empty())
        {
            QString front = filenames.first();
            filenames.pop_front();

            QString fname = QFileInfo(front).fileName();
            //kDebug(30009) << "Loading " << fname << " of type " << type();
            // XXX: Don't load resources with the same filename. Actually, we should look inside
            //      the resource to find out whether they are really the same, but for now this
            //      will prevent the same brush etc. showing up twice.
            if (uniqueFiles.empty() || uniqueFiles.indexOf(fname) == -1) {
                loadLock.lock();
                uniqueFiles.append(fname);
                T* resource = createResource(front);
                if (resource->load() && resource->valid())
                {
                    m_resources.append(resource);
                    notifyResourceAdded(resource);
                    Q_CHECK_PTR(resource);
                }
                else {
                    delete resource;
                }
                loadLock.unlock();
            }
        }
        kDebug(30009) << "done loading  resources for type " << type();
    }


    /// Adds an already loaded resource to the server
    bool addResource(T* resource) {
        if (!resource->valid()) {
            kWarning(30009) << "Tried to add an invalid resource!";
            return false;
        }
        if( ! resource->save() )
            return false;

        m_resources.append(resource);
        notifyResourceAdded(resource);

        return true;
    }

    /// Remove a resource from resourceserver and hard disk
    bool removeResource(T* resource) {
        int index = m_resources.indexOf( resource );
        if( index < 0 )
            return false;

        QFile file( resource->filename() );

        if( ! file.remove() )
            return false;

        notifyRemovingResource(resource);

        m_resources.removeAt( index );
        delete resource;
        return true;
    }

    QList<T*> resources() {
        loadLock.lock();
        QList<T*> resourceList = m_resources;
        loadLock.unlock();
        return resourceList;
    }

    /// Returns path where to save user defined and imported resources to
    virtual QString saveLocation() {
        return KGlobal::mainComponent().dirs()->saveLocation(type().toAscii());
    }

    T* importResource( const QString & filename ) {
        QFileInfo fi( filename );
        if( fi.exists() == false )
            return 0;

        T* resource = createResource( filename );
        resource->load();
        if(!resource->valid()){
            kWarning(30009) << "Import failed! Resource is not valid";
            delete resource;
            return 0;
         }

         Q_ASSERT(!resource->defaultFileExtension().isEmpty());
         Q_ASSERT(!saveLocation().isEmpty());

        QString newFilename = saveLocation() + fi.baseName() + resource->defaultFileExtension();
        resource->setFilename(newFilename);
        if(!addResource(resource)) {
            delete resource;
            return 0;
        }

        return resource;
    }

    /**
     * Addes an observer to the server
     * @param observer the observer to be added
     * @param notifyLoadedResources determines if the observer should be notified about the already loaded resources 
     */
    void addObserver(KoResourceServerObserver<T>* observer, bool notifyLoadedResources = true)
    {
        loadLock.lock();
        if(observer && !m_observers.contains(observer)) {
            m_observers.append(observer);

            if(notifyLoadedResources) {
                foreach(T* resource, m_resources)
                    observer->resourceAdded(resource);
            }
        }
        loadLock.unlock();
    }

    void removeObserver(KoResourceServerObserver<T>* observer)
    {
        int index = m_observers.indexOf( observer );
        if( index < 0 )
            return;

        m_observers.removeAt( index );
    }

protected:
    virtual T* createResource( const QString & filename ) { return new T(filename); }

    void notifyResourceAdded(T* resource)
    {
        foreach(KoResourceServerObserver<T>* observer, m_observers) {
            observer->resourceAdded(resource);
        }
    }

    void notifyRemovingResource(T* resource)
    {
        foreach(KoResourceServerObserver<T>* observer, m_observers)
            observer->removingResource(resource);
    }


private:
    QList<T*> m_resources;
    QList<KoResourceServerObserver<T>*> m_observers;

};

#endif // KORESOURCESERVER_H
