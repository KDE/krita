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

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QFileInfo>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include "KoResource.h"
#include "KoResourceServerObserver.h"

#include "kowidgets_export.h"

#include <KDebug>

class KoResource;

/**
 * KoResourceServerBase is the base class of all resource servers
 */
class KOWIDGETS_EXPORT KoResourceServerBase {

public:
    /**
    * Constructs a KoResourceServerBase
    * @param resource type, has to be the same as used by KStandardDirs
    * @param extensions the file extensions separate by ':', e.g. "*.kgr:*.svg:*.ggr"
    */
    KoResourceServerBase(const QString& type, const QString& extensions)
        : m_type(type)
        , m_extensions(extensions)
        , m_cancelled(false)
    {
    }

    virtual ~KoResourceServerBase() {}

    virtual void loadResources(QStringList filenames) = 0;
    QString type() { return m_type; }

    /**
    * File extensions for resources of the server
    * @returns the file extensions separated by ':', e.g. "*.kgr:*.svg:*.ggr"
    */
    QString extensions() { return m_extensions; }

    void cancel() { m_cancelled = true; }

private:
    QString m_type;
    QString m_extensions;

protected:

    bool   m_cancelled;
    QMutex m_loadLock;

};

/**
 * KoResourceServer manages the resources of one type. It stores, loads and saves the resources.
 * To keep track of changes the server can be observed with a KoResourceServerObserver
 */
template <class T> class KoResourceServer : public KoResourceServerBase {

public:
    KoResourceServer(const QString& type, const QString& extensions)
        : KoResourceServerBase(type, extensions)

        {
        }

    virtual ~KoResourceServer()
        {
            qDeleteAll(m_resources);
        }

   /**
     * Loads a set of resources and adds them to the resource server.
     * If a filename appears twice the resource will only be added once. Resources that can't
     * be loaded or and invalid aren't added to the server.
     * @param filenames list of filenames to be loaded
     */
    void loadResources(QStringList filenames) {
        kDebug(30009) << "loading  resources for type " << type();
        QStringList uniqueFiles;

        while (!filenames.empty() && !m_cancelled)
        {
            QString front = filenames.first();
            filenames.pop_front();

            QString fname = QFileInfo(front).fileName();

            //kDebug(30009) << "Loading " << fname << " of type " << type();
            // XXX: Don't load resources with the same filename. Actually, we should look inside
            //      the resource to find out whether they are really the same, but for now this
            //      will prevent the same brush etc. showing up twice.
            if (uniqueFiles.empty() || uniqueFiles.indexOf(fname) == -1) {
                m_loadLock.lock();
                uniqueFiles.append(fname);
                QList<T*> resources = createResources(front);
                foreach(T* resource, resources) {
                    if (resource->load() && resource->valid())
                    {
                        m_resourcesByFilename[front] = resource;

                        if ( resource->name().isNull() ) {
                            resource->setName( fname );
                        }
                        m_resourcesByName[resource->name()] = resource;
                        m_resources.append(resource);

                        notifyResourceAdded(resource);
                        Q_CHECK_PTR(resource);
                    }
                    else {
                        delete resource;
                    }
                }
                m_loadLock.unlock();
            }
        }
        kDebug(30009) << "done loading  resources for type " << type();
    }


    /// Adds an already loaded resource to the server
    bool addResource(T* resource, bool save = true) {
        if (!resource->valid()) {
            kWarning(30009) << "Tried to add an invalid resource!";
            return false;
        }
        if( save && ! resource->save()) {
            kWarning(30009) << "Could not save resource!";
            return false;
        }

        Q_ASSERT( !resource->filename().isEmpty() || !resource->name().isEmpty() );
        if ( resource->filename().isEmpty() ) {
            resource->setFilename( resource->name() );
        }
        else if ( resource->name().isEmpty() ) {
            resource->setName( resource->filename() );
        }

        m_resourcesByFilename[resource->filename()] = resource;
        m_resourcesByName[resource->name()] = resource;
        m_resources.append(resource);

        notifyResourceAdded(resource);

        return true;
    }

    /// Remove a resource from resourceserver and hard disk
    bool removeResource(T* resource) {
        if ( !m_resourcesByFilename.contains( resource->filename() ) ) {
            return false;
        }

        bool removedFromDisk = true;

        QFile file( resource->filename() );
        if( ! file.remove() ) {

            // Don't do anything, it's probably write protected. In
            // //future, we should store in config which read-only
            // //resources the user has removed and blacklist them on
            // app-start. But if we cannot remove a resource from the
            // disk, remove it from the chooser at least.

            removedFromDisk = false;
            kWarning(30009) << "Could not remove resource!";
        }

        if (removedFromDisk) {
            m_resourcesByName.remove(resource->name());
            m_resourcesByFilename.remove(resource->filename());
            m_resources.removeAt(m_resources.indexOf(resource));
            notifyRemovingResource(resource);
            delete resource;
        } else {
            // TODO: save blacklist to config file and load it again on next start
            m_resourceBlackList << resource;
        }

        return true;
    }

    QList<T*> resources() {
        m_loadLock.lock();
        QList<T*> resourceList = m_resources;
        foreach(T* r, m_resourceBlackList) {
            resourceList.removeOne(r);
        }
        m_loadLock.unlock();
        return resourceList;
    }

    /// Returns path where to save user defined and imported resources to
    virtual QString saveLocation() {
        return KGlobal::mainComponent().dirs()->saveLocation(type().toAscii());
    }

    /**
     * Creates a new resourcea from a given file and adds them to the resource server
     * The base implementation does only load one resource per file, override to implement collections
     * @param filename file name of the resource file to be imported
     */
    virtual void importResourceFile( const QString & filename ) {
        QFileInfo fi( filename );
        if( fi.exists() == false )
            return;

        T* resource = createResource( filename );
        resource->load();
        if(!resource->valid()){
            kWarning(30009) << "Import failed! Resource is not valid";
            delete resource;
            return;
         }

         Q_ASSERT(!resource->defaultFileExtension().isEmpty());
         Q_ASSERT(!saveLocation().isEmpty());

        QString newFilename = saveLocation() + fi.baseName() + resource->defaultFileExtension();
        QFileInfo fileInfo(newFilename);

        int i = 1;
        while (fileInfo.exists()) {
            fileInfo.setFile(saveLocation() + fi.baseName() + QString("%1").arg(i) + resource->defaultFileExtension());
            i++;
        }
        resource->setFilename(fileInfo.filePath());
        if(!addResource(resource)) {
            delete resource;
        }
    }

    /**
     * Addes an observer to the server
     * @param observer the observer to be added
     * @param notifyLoadedResources determines if the observer should be notified about the already loaded resources
     */
    void addObserver(KoResourceServerObserver<T>* observer, bool notifyLoadedResources = true)
    {
        m_loadLock.lock();
        if(observer && !m_observers.contains(observer)) {
            m_observers.append(observer);

            if(notifyLoadedResources) {
                foreach(T* resource, m_resourcesByFilename) {
                    observer->resourceAdded(resource);
                }
            }
        }
        m_loadLock.unlock();
    }

    /**
     * Removes an observer from the server
     * @param observer the observer to be removed
     */
    void removeObserver(KoResourceServerObserver<T>* observer)
    {
        int index = m_observers.indexOf( observer );
        if( index < 0 )
            return;

        m_observers.removeAt( index );
    }

    T* getResourceByFilename( const QString& filename )
    {
        if ( !m_resourcesByFilename.contains( filename ) ) {
            return 0;
        }

        return m_resourcesByFilename[filename];
    }


    T* getResourceByName( const QString& name )
    {
        if ( !m_resourcesByName.contains( name ) ) {
            return 0;
        }

        return m_resourcesByName[name];
    }

    /**
     * Call after changing the content of a resource;
     * Notifies the connected views.
     */
    void updateResource( T* resource )
    {
        notifyResourceChanged(resource);
    }

protected:

    /**
     * Create one or more resources from a single file. By default one resource is created.
     * Overide to create more resources from the file.
     * @param filename the filename of the resource or resource collection
     */
    virtual QList<T*> createResources( const QString & filename )
    {
        QList<T*> createdResources;
        createdResources.append(createResource(filename));
        return createdResources;
    }
    
    virtual T* createResource( const QString & filename ) { return new T(filename); }

    void notifyResourceAdded(T* resource)
    {
        foreach(KoResourceServerObserver<T>* observer, m_observers) {
            observer->resourceAdded(resource);
        }
    }

    void notifyRemovingResource(T* resource)
    {
        foreach(KoResourceServerObserver<T>* observer, m_observers) {
            observer->removingResource(resource);
        }
    }

    void notifyResourceChanged(T* resource)
    {
        foreach(KoResourceServerObserver<T>* observer, m_observers) {
            observer->resourceChanged(resource);
        }
    }

private:

    QHash<QString, T*> m_resourcesByName;
    QHash<QString, T*> m_resourcesByFilename;
    QList<T*> m_resourceBlackList;
    QList<T*> m_resources; ///< list of resources in order of addition
    QList<KoResourceServerObserver<T>*> m_observers;

};

#endif // KORESOURCESERVER_H
