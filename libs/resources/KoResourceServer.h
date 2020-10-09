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
#include <QApplication>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QDir>
#include <QTemporaryFile>

#include "KoResource.h"
#include "KoResourceServerObserver.h"
#include "KoResourcePaths.h"
#include "ksharedconfig.h"

#include <KisResourceModel.h>
#include <KisTagModel.h>
#include <kis_assert.h>
#include <kis_debug.h>

#include <ResourceDebug.h>

class KoResource;

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
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
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
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
        }

        return m_resourceModel;
    }

    /// Return the first resource available
    QSharedPointer<T> firstResource() const
    {

        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
        }

        if (QThread::currentThread() != qApp->thread()) {
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
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
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
        }

        if (!resource->valid()) {
            warnResource << "Tried to add an invalid resource!";
            return false;
        }

        if (m_resourceModel->addResource(resource, save ? resource->storageLocation() : "memory")) {
            notifyResourceAdded(resource);
            return true;
        }

        return false;
    }

    /// Remove a resource from Resource Server but not from a file
    bool removeResourceFromServer(QSharedPointer<T> resource){

        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
        }

        if (m_resourceModel->setResourceInactive(m_resourceModel->indexForResource(resource))) {
            notifyRemovingResource(resource);
            return true;
        }
        return false;
    }

    QList<QSharedPointer<T>> resources() {

        qDebug() << "KoResourceServer::resources()" << m_type;

        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
        }

        KIS_SAFE_ASSERT_RECOVER_NOOP(m_type != "paintoppresets");
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

        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
        }

        return m_resourceModel->importResourceFile(filename);
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
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
        }


        //qDebug() << "resourceByFilename" << filename;
        if (filename.isEmpty() || filename.isNull()) {
            return 0;
        }
        return m_resourceModel->resourceForFilename(filename).dynamicCast<T>();
    }


    QSharedPointer<T> resourceByName(const QString& name) const
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
        }


        //qDebug() << "resourceByName" << name;
        if (name.isEmpty() || name.isNull()) {
            return 0;
        }
        return m_resourceModel->resourceForName(name).dynamicCast<T>();

    }

    QSharedPointer<T> resourceByMD5(const QByteArray& md5) const
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
        }


        //qDebug() << "resourceByMD5" << md5.toHex();
        if (md5.isEmpty() || md5.isNull()) {
            return 0;
        }
        return m_resourceModel->resourceForMD5(md5).dynamicCast<T>();
    }

    /**
     * Call after changing the content of a resource;
     * Notifies the connected views.
     */
    void updateResource(QSharedPointer<T> resource)
    {


        KIS_SAFE_ASSERT_RECOVER_NOOP(QThread::currentThread() == qApp->thread());
        if (QThread::currentThread() != qApp->thread()) {
            Q_FOREACH(const QString &s, kisBacktrace().split('\n')) {
                qDebug() << s;
            }
        }
        m_resourceModel->updateResource(resource);
        notifyResourceChanged(resource);
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
