/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
    Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KORESOURCESERVER_H
#define KORESOURCESERVER_H

#include <QMutex>
#include <QString>
#include <QStringList>
#include <QList>
#include <QFileInfo>
#include <QDir>

#include <QTemporaryFile>
#include <QDomDocument>
#include "KoResource.h"
#include "KoResourceServerObserver.h"
#include "KoResourcePaths.h"
#include <KisResourceModel.h>
#include <KisResourceModelProvider.h>

#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "kritawidgets_export.h"
#include "WidgetsDebug.h"

class KoResource;

/**
 * KoResourceServerBase is the base class of all resource servers
 */
class KRITAWIDGETS_EXPORT KoResourceServerBase {

public:
    /**
    * Constructs a KoResourceServerBase
    * @param type type, has to be the same as used by KoResourcePaths
    */
    KoResourceServerBase(const QString& type)
        : m_resourceModel(KisResourceModelProvider::resourceModel(type))
        , m_type(type)
    {
        qDebug() << "Creating KoResourceServerBase" << m_type;
    }

    virtual ~KoResourceServerBase() {}

    virtual int resourceCount() const = 0;

protected:

    KisResourceModel *m_resourceModel {0};
    QString m_type;
};

/**
 * KoResourceServer manages the resources of one type. It stores,
 * loads and saves the resources.  To keep track of changes the server
 * can be observed with a KoResourceServerObserver
 */

template <class T>
class KoResourceServer : public KoResourceServerBase
{
public:
    typedef KoResourceServerObserver<T> ObserverType;

    KoResourceServer(const QString& type)
        : KoResourceServerBase(type)
    {
    }

    ~KoResourceServer() override
    {
        Q_FOREACH (ObserverType* observer, m_observers) {
            observer->unsetResourceServer();
        }
    }

    int resourceCount() const override {
        return m_resourceModel->rowCount();
    }

    /// Adds an already loaded resource to the server
    bool addResource(QSharedPointer<T> resource, bool save = true) {
        if (!resource->valid()) {
            warnWidgets << "Tried to add an invalid resource!";
            return false;
        }

        if (save) {
            QFileInfo fileInfo(resource->filename());

            QDir d(fileInfo.path());
            if (!d.exists()) {
                d.mkdir(fileInfo.path());
            }

            if (fileInfo.exists()) {
                QString filename = fileInfo.path() + "/" + fileInfo.baseName() + "XXXXXX" + "." + fileInfo.suffix();
                debugWidgets << "fileName is " << filename;
                QTemporaryFile file(filename);
                if (file.open()) {
                    debugWidgets << "now " << file.fileName();
                    resource->setFilename(file.fileName());
                }
            }

            if (!resource->save()) {
                warnWidgets << "Could not save resource!";
                return false;
            }
        }

        Q_ASSERT(!resource->filename().isEmpty() || !resource->name().isEmpty());
        if (resource->filename().isEmpty()) {
            resource->setFilename(resource->name());
        }
        else if (resource->name().isEmpty()) {
            resource->setName(resource->filename());
        }

        notifyResourceAdded(resource);

        return true;
    }

    /// Remove a resource from Resource Server but not from a file
    bool removeResourceFromServer(QSharedPointer<T> resource){
        return true;
    }

    /// Remove a resource from the resourceserver and blacklist it

    bool removeResourceAndBlacklist(QSharedPointer<T> resource) {
        return true;
    }

    QList<QSharedPointer<T>> resources() {
        QList<QSharedPointer<T>> resourceList;
        for (int row = 0; row < m_resourceModel->rowCount(); ++row) {
            resourceList << m_resourceModel->resourceForIndex(m_resourceModel->index(row, 0)).dynamicCast<T>();
        }
        return resourceList;
    }

    /// Returns path where to save user defined and imported resources to
    virtual QString saveLocation() {
        return KoResourcePaths::saveLocation(m_type.toLatin1());
    }

    /**
     * Creates a new resource from a given file and adds them to the resource server
     * The base implementation does only load one resource per file, override to implement collections
     * @param filename file name of the resource file to be imported
     * @param fileCreation decides whether to create the file in the saveLocation() directory
     */
    virtual bool importResourceFile(const QString & filename , bool fileCreation=true) {

//        QFileInfo fi(filename);
//        if (!fi.exists())
//            return false;
//        if ( fi.size() == 0)
//            return false;

//        QSharedPointer<T> resource = createResource( filename );
//        resource->load();
//        if (!resource->valid()) {
//            warnWidgets << "Import failed! Resource is not valid";

//            return false;

//        }

//        if (fileCreation) {
//            Q_ASSERT(!resource->defaultFileExtension().isEmpty());
//            Q_ASSERT(!saveLocation().isEmpty());

//            QString newFilename = saveLocation() + fi.baseName() + resource->defaultFileExtension();
//            QFileInfo fileInfo(newFilename);

//            int i = 1;
//            while (fileInfo.exists()) {
//                fileInfo.setFile(saveLocation() + fi.baseName() + QString("%1").arg(i) + resource->defaultFileExtension());
//                i++;
//            }
//            resource->setFilename(fileInfo.filePath());
//        }
        return true;
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
    void addObserver(ObserverType* observer, bool notifyLoadedResources = true)
    {
        if (observer && !m_observers.contains(observer)) {
            m_observers.append(observer);

            if(notifyLoadedResources) {
//                Q_FOREACH (QSharedPointer<T> resource, ByFilename) {
//                    observer->resourceAdded(resource);

//                }
            }
        }
    }

    /**
     * Removes an observer from the server
     * @param observer the observer to be removed
     */
    void removeObserver(ObserverType* observer)
    {
        int index = m_observers.indexOf( observer );
        if( index < 0 )
            return;

        m_observers.removeAt( index );
    }

    QSharedPointer<T> resourceByFilename(const QString& /*filename*/) const
    {
//        if (m_resourcesByFilename.contains(filename)) {
//            return m_resourcesByFilename[filename];
//        }
        return 0;
    }


    QSharedPointer<T> resourceByName( const QString& /*name */) const
    {
//        if (m_resourcesByName.contains(name)) {
//            return m_resourcesByName[name];
//        }
        return 0;
    }

    QSharedPointer<T> resourceByMD5(const QByteArray& /*md5*/) const {
//        return m_resourcesByMd5.value(md5);
        return 0;
    }

    /**
     * Call after changing the content of a resource;
     * Notifies the connected views.
     */
    void updateResource(QSharedPointer<T> resource)
    {
        notifyResourceChanged(resource);
    }


    // don't use these method directly since it doesn't update views!
    void addTag(KoResourceSP resource, const QString& tag)
    {
//        m_tagStore->addTag(resource, tag);
    }

    // don't use these method directly since it doesn't update views!
    void delTag(KoResourceSP resource, const QString& tag)
    {
//        m_tagStore->delTag(resource, tag);
    }


    QStringList assignedTagsList(KoResourceSP resource) const
    {
        return QStringList(); //m_tagStore->assignedTagsList(resource);
    }

    /// Return the currently stored resources in alphabetical order, overwrite for customized sorting
    virtual QList<QSharedPointer<T>> sortedResources()
    {
        QMap<QString, QSharedPointer<T>> sortedNames;
//        Q_FOREACH (const QString &name, m_resourcesByName.keys()) {
//            sortedNames.insert(name.toLower(), m_resourcesByName[name]);
//        }
        return sortedNames.values();
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

};

#endif // KORESOURCESERVER_H
