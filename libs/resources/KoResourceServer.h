/*  This file is part of the KDE project

    SPDX-FileCopyrightText: 1999 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2003 Patrick Julien <freak@codepimps.org>
    SPDX-FileCopyrightText: 2005 Sven Langkamp <sven.langkamp@gmail.com>
    SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
    SPDX-FileCopyrightText: 2011 Srikanth Tiyyagura <srikanth.tulasiram@gmail.com>
    SPDX-FileCopyrightText: 2013 Sascha Suelzer <s.suelzer@gmail.com>
    SPDX-FileCopyrightText: 2003-2019 Boudewijn Rempt <boud@valdyas.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef KORESOURCESERVER_H
#define KORESOURCESERVER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QFileInfo>
#include <QApplication>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QDir>
#include <QTemporaryFile>

#include "KoResource.h"
#include "KoResourcePaths.h"
#include "ksharedconfig.h"

#include <KisGlobalResourcesInterface.h>
#include <KisResourceLocator.h>
#include <KisResourceModel.h>
#include <KisTagModel.h>
#include <kis_assert.h>
#include <kis_debug.h>

#include <ResourceDebug.h>

class KoResource;

template <class T>
class KoResourceServerObserver
{
public:
    virtual ~KoResourceServerObserver() {}

    virtual void unsetResourceServer() = 0;

    /**
     * Will be called by the resource server after a resource is added
     * @param resource the added resource
     */
    virtual void resourceAdded(QSharedPointer<T> resource) = 0;

    /**
     * Will be called by the resource server before a resource will be removed
     * @param resource the resource which is going to be removed
     */
    virtual void removingResource(QSharedPointer<T> resource) = 0;

    /**
     * Will be called by the resource server when a resource is changed
     * @param resource the resource which is going to be removed
     */
    virtual void resourceChanged(QSharedPointer<T> resource) = 0;


};

/**
 * KoResourceServer is a shim around KisResourceModel. It knows
 * nothing by its own, and does nothing on its own. It can only
 * be used in the gui thread.
 */
template <class T>
class KoResourceServer
{
public:

    typedef KoResourceServerObserver<T> ObserverType;

    KoResourceServer(const QString& type)
        : m_resourceModel(new KisResourceModel(type))
        , m_tagModel(new KisTagModel(type))
        , m_type(type)
    {
        Q_ASSERT(!type.isEmpty());
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            qDebug().noquote() << kisBacktrace();
        }

    }

    virtual ~KoResourceServer()
    {
        delete m_resourceModel;
        delete m_tagModel;
        Q_FOREACH (ObserverType* observer, m_observers) {
            observer->unsetResourceServer();
        }
    }

    /// @return the active resource model
    KisResourceModel *resourceModel() const
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            qDebug().noquote() << kisBacktrace();
        }

        return m_resourceModel;
    }

    /// Return the first resource available
    QSharedPointer<T> firstResource() const
    {

        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            qDebug().noquote() << kisBacktrace();
        }

        return m_resourceModel->resourceForIndex(m_resourceModel->index(0, 0)).dynamicCast<T>();
    }

    int resourceCount() const {

        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        return m_resourceModel->rowCount();
    }

    /// Adds an already loaded resource to the server
    bool addResource(QSharedPointer<T> resource, bool save = true) {

        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            qDebug().noquote() << kisBacktrace();
        }

        if (!resource || !resource->valid()) {
            warnResource << "Tried to add an invalid resource!";
            return false;
        }

        if (m_resourceModel->addResource(resource, save ? QString() : "memory")) {
            notifyResourceAdded(resource);
            return true;
        }

        return false;
    }

    /// Remove a resource from Resource Server but not from a file
    bool removeResourceFromServer(QSharedPointer<T> resource){

        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            qDebug().noquote() << kisBacktrace();
        }

        if (m_resourceModel->setResourceInactive(m_resourceModel->indexForResource(resource))) {
            notifyRemovingResource(resource);
            return true;
        }
        return false;
    }

    /// Returns path where to save user defined and imported resources to
    QString saveLocation() {
        return KisResourceLocator::instance()->resourceLocationBase() + m_type;
    }

    /**
     * Creates a new resource from a given file and adds them to the resource server
     * The base implementation does only load one resource per file, override to implement collections
     * @param filename file name of the resource file to be imported
     * @param fileCreation decides whether to create the file in the saveLocation() directory
     */
    KoResourceSP importResourceFile(const QString &filename, const bool allowOverwrite)
    {

        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            qDebug().noquote() << kisBacktrace();
        }

        return m_resourceModel->importResourceFile(filename, allowOverwrite);
    }

    /// Removes the resource file from the resource server
    void removeResourceFile(const QString & filename)
    {
        QFileInfo fi(filename);

        QSharedPointer<T> resource = resourceByFilename(fi.fileName());
        if (!resource) {
            warnResource << "Resource file do not exist ";
            return;
        }
        removeResourceFromServer(resource);
    }

    /**
     * Adds an observer to the server
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

private:

    QSharedPointer<T> resourceByFilename(const QString& filename) const
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            qDebug().noquote() << kisBacktrace();
        }


        //qDebug() << "resourceByFilename" << filename;
        if (filename.isEmpty() || filename.isNull()) {
            return nullptr;
        }
        QVector<KoResourceSP> resources = m_resourceModel->resourcesForFilename(filename);
        if (resources.size() > 0) {
            return resources.first().dynamicCast<T>();
        }
        return nullptr;
    }


    QSharedPointer<T> resourceByName(const QString& name) const
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            qDebug().noquote() << kisBacktrace();
        }

        if (name.isEmpty() || name.isNull()) {
            return nullptr;
        }

        QVector<KoResourceSP> resources = m_resourceModel->resourcesForName(name);
        if (resources.size() > 0) {
            return resources.first().dynamicCast<T>();
        }

        return nullptr;

    }

    QSharedPointer<T> resourceByMD5(const QString& md5) const
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            qDebug().noquote() << kisBacktrace();
        }
        if (md5.isEmpty() || md5.isNull()) {
            return nullptr;
        }
        QVector<KoResourceSP> resources = m_resourceModel->resourcesForMD5(md5);

        if (resources.size() > 0) {
            return resources.first().dynamicCast<T>();
        }
        return nullptr;
    }

public:

    /**
     * @brief resource retrieves a resource. If the md5sum is not empty, the resource
     * will only be retrieved if a resource with that md5sum exists. If it is empty,
     * a fallback to filename or name is possible.
     * @param md5 This is the hex-encoded md5sum as stored in e.g. configuration objects
     * @param fileName A filename without the path
     * @param name The name of the resource
     * @return a resource, or nullptr
     */
    QSharedPointer<T> resource(const QString &md5, const QString &fileName, const QString &name)
    {
        KoResourceSP res = KisGlobalResourcesInterface::instance()->source(m_type).resource(md5, fileName, name);
        return res.dynamicCast<T>();
    }



    /**
     * Call after changing the content of a resource and saving it;
     * Notifies the connected views.
     */
    bool updateResource(QSharedPointer<T> resource)
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            qDebug().noquote() << kisBacktrace();
        }
        bool result = m_resourceModel->updateResource(resource);
        notifyResourceChanged(resource);
        return result;
    }

    /**
     * Reloads the resource from the persistent storage
     */
    bool reloadResource(QSharedPointer<T> resource)
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            qDebug().noquote() << kisBacktrace();
        }
        bool result = m_resourceModel->reloadResource(resource);
        notifyResourceChanged(resource);

        return result;
    }

    QVector<KisTagSP> assignedTagsList(KoResourceSP resource) const
    {
        if (resource.isNull()) {
            return QVector<KisTagSP>();
        }
        return m_resourceModel->tagsForResource(resource->resourceId());
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
    KisResourceModel *m_resourceModel {0};
    KisTagModel *m_tagModel {0};
    QString m_type;
};

#endif // KORESOURCESERVER_H
