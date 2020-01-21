/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
    Copyright (C) 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
    Copyright (c) 2013 Sascha Suelzer <s.suelzer@gmail.com>
    Copyright (c) 2003-2019 Boudewijn Rempt <boud@valdyas.org>

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

#ifndef KORESOURCESERVER_H
#define KORESOURCESERVER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QFileInfo>
#include <QDir>
#include <QApplication>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>

#include <QTemporaryFile>
#include <QDomDocument>
#include "KoResource.h"
#include "KoResourceServerObserver.h"
#include "KoResourcePaths.h"
#include <KisResourceModel.h>
#include <KisResourceModelProvider.h>
#include <KisTagModelProvider.h>
#include <KisTagsResourcesModelProvider.h>
#include <KisTagsResourcesModel.h>

#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "kritawidgets_export.h"
#include "WidgetsDebug.h"

class KoResource;

/**
 * KoResourceServer manages the resources of one type. It stores,
 * loads and saves the resources. To keep track of changes the server
 * can be observed with a KoResourceServerObserver
 */
template <class T>
class KoResourceServer
{
public:

    typedef KoResourceServerObserver<T> ObserverType;

    KoResourceServer(const QString& type)
        : m_resourceModel(KisResourceModelProvider::resourceModel(type))
        , m_tagModel(KisTagModelProvider::tagModel(type))
        , m_tagsResourcesModel(KisTagsResourcesModelProvider::getModel(type))
        , m_type(type)
    {
        //Q_ASSERT(QThread::currentThread() == qApp->thread());
    }

    virtual ~KoResourceServer()
    {
        Q_FOREACH (ObserverType* observer, m_observers) {
            observer->unsetResourceServer();
        }
    }

    /// @return the active resource model
    KisResourceModel *resourceModel() const
    {
        QMutexLocker l(&m_mutex);
        //Q_ASSERT(QThread::currentThread() == qApp->thread());
        return m_resourceModel;
    }

    /// Return the first resource available
    QSharedPointer<T> firstResource() const
    {
        QMutexLocker l(&m_mutex);
        //Q_ASSERT(QThread::currentThread() == qApp->thread());
        return m_resourceModel->resourceForIndex(m_resourceModel->index(0, 0)).dynamicCast<T>();
    }

    int resourceCount() const {
        QMutexLocker l(&m_mutex);
        //Q_ASSERT(QThread::currentThread() == qApp->thread());
        return m_resourceModel->rowCount();
    }

    /// Adds an already loaded resource to the server
    bool addResource(QSharedPointer<T> resource, bool save = true) {
        QMutexLocker l(&m_mutex);
        //Q_ASSERT(QThread::currentThread() == qApp->thread());
        if (!resource->valid()) {
            warnWidgets << "Tried to add an invalid resource!";
            return false;
        }

        if (m_resourceModel->addResource(resource, save)) {
            notifyResourceAdded(resource);
            return true;
        }

        return false;
    }

    /// Remove a resource from Resource Server but not from a file
    bool removeResourceFromServer(QSharedPointer<T> resource){
        QMutexLocker l(&m_mutex);
        //Q_ASSERT(QThread::currentThread() == qApp->thread());
        if (m_resourceModel->removeResource(resource)) {
            notifyRemovingResource(resource);
            return true;
        }
        return false;
    }

    QList<QSharedPointer<T>> resources() {
        QMutexLocker l(&m_mutex);
        qDebug() << "KoResourceServer::resources()" << m_type;
        //Q_ASSERT(QThread::currentThread() == qApp->thread());
        //Q_ASSERT(m_type != "paintoppresets");
        QList<QSharedPointer<T>> resourceList;
        for (int row = 0; row < m_resourceModel->rowCount(); ++row) {
            resourceList << m_resourceModel->resourceForIndex(m_resourceModel->index(row, 0)).dynamicCast<T>();
        }
        return resourceList;
    }

    /// Returns path where to save user defined and imported resources to
    QString saveLocation() {
        return KoResourcePaths::saveLocation(m_type.toLatin1());
    }

    /**
     * Creates a new resource from a given file and adds them to the resource server
     * The base implementation does only load one resource per file, override to implement collections
     * @param filename file name of the resource file to be imported
     * @param fileCreation decides whether to create the file in the saveLocation() directory
     */
    bool importResourceFile(const QString &filename)
    {
        QMutexLocker l(&m_mutex);
        //Q_ASSERT(QThread::currentThread() == qApp->thread());
        return m_resourceModel->importResourceFile(filename);
    }

    /// Removes the resource file from the resource server
    void removeResourceFile(const QString & filename)
    {
        QFileInfo fi(filename);

        QSharedPointer<T> resource = resourceByFilename(fi.fileName());
        if (!resource) {
            warnWidgets << "Resource file do not exist ";
            return;
        }
        removeResourceFromServer(resource);
    }

    /**
     * Addes an observer to the server
     * @param observer the observer to be added
     * @param notifyLoadedResources determines if the observer should be notified about the already loaded resources
     */
    void addObserver(ObserverType* observer)
    {
        if (observer && !m_observers.contains(observer)) {
            m_observers.append(observer);
        }
    }

    /**
     * Removes an observer from the server
     * @param observer the observer to be removed
     */
    void removeObserver(ObserverType* observer)
    {
        int index = m_observers.indexOf(observer);
        if (index < 0) {
            return;
        }
        m_observers.removeAt( index );
    }

    QSharedPointer<T> resourceByFilename(const QString& filename) const
    {
        QMutexLocker l(&m_mutex);
        qDebug() << "resourceByFilename" << filename;
//        if (m_resourcesByFilename.contains(filename)) {
//            return m_resourcesByFilename[filename];
//        }
        if (filename.isEmpty() || filename.isNull()) {
            return 0;
        }
        return m_resourceModel->resourceForFilename(filename).dynamicCast<T>();
    }


    QSharedPointer<T> resourceByName(const QString& name) const
    {
        QMutexLocker l(&m_mutex);
        qDebug() << "resourceByName" << name;
        if (name.isEmpty() || name.isNull()) {
            return 0;
        }
        return m_resourceModel->resourceForName(name).dynamicCast<T>();
//        if (m_resourcesByName.contains(name)) {
//            return m_resourcesByName[name];
//        }
        //return 0;
    }

    QSharedPointer<T> resourceByMD5(const QByteArray& md5) const
    {
        //This needs MD5 sums to be implemented properly...
        QMutexLocker l(&m_mutex);
        qDebug() << "resourceByMD5" << md5;
//        return m_resourcesByMd5.value(md5);
        return 0;
    }

    /**
     * Call after changing the content of a resource;
     * Notifies the connected views.
     */
    void updateResource(QSharedPointer<T> resource)
    {
        QMutexLocker l(&m_mutex);
        //Q_ASSERT(QThread::currentThread() == qApp->thread());
        m_resourceModel->updateResource(resource);
        notifyResourceChanged(resource);
    }

    QVector<KisTagSP> assignedTagsList(KoResourceSP resource) const
    {
        return m_resourceModel->tagsForResource(resource->resourceId());
    }

    QVector<KoResourceSP> resourcesForTag(KisTagSP tag) const
    {
        return m_tagsResourcesModel->resourcesForTag(tag->id());
    }

protected:

    void notifyResourceAdded(QSharedPointer<T> resource)
    {
        Q_FOREACH (ObserverType* observer, m_observers) {
            observer->resourceAdded(resource);
        }
    }

    void notifyRemovingResource(QSharedPointer<T> resource)
    {
        Q_FOREACH (ObserverType* observer, m_observers) {
            observer->removingResource(resource);
        }
    }

    void notifyResourceChanged(QSharedPointer<T> resource)
    {
        Q_FOREACH (ObserverType* observer, m_observers) {
            observer->resourceChanged(resource);
        }
    }

private:

    QList<ObserverType*> m_observers;
    KisTagModel *m_tagModel {0};
    KisTagsResourcesModel *m_tagsResourcesModel {0};
    KisResourceModel *m_resourceModel {0};
    QString m_type;
    mutable QMutex m_mutex;
};

#endif // KORESOURCESERVER_H
