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
#include <QMutexLocker>
#include <QString>
#include <QStringList>
#include <QList>
#include <QFileInfo>
#include <QDir>
#include <QMultiHash>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include <QXmlStreamReader>
#include <QTemporaryFile>
#include <QDomDocument>
#include "KoResource.h"
#include "KoResourceServerPolicies.h"
#include "KoResourceServerObserver.h"
#include "KoResourceTagStore.h"

#include "kowidgets_export.h"

#include <kdebug.h>

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
    {
    }

    virtual ~KoResourceServerBase() {}

    virtual int resourceCount() const = 0;
    virtual void loadResources(QStringList filenames) = 0;
    virtual QStringList blackListedFiles() const = 0;
    virtual QStringList queryResources(const QString &query) const = 0;
    QString type() const { return m_type; }

    /**
    * File extensions for resources of the server
    * @returns the file extensions separated by ':', e.g. "*.kgr:*.svg:*.ggr"
    */
    QString extensions() const { return m_extensions; }

    QStringList fileNames() const
    {
        QStringList extensionList = m_extensions.split(':');
        QStringList fileNames;

        foreach (const QString &extension, extensionList) {
            fileNames += KGlobal::dirs()->findAllResources(type().toLatin1(), extension, KStandardDirs::Recursive | KStandardDirs::NoDuplicates);

        }
        return fileNames;
    }

protected:

    friend class KoResourceTagStore;
    virtual KoResource *byMd5(const QByteArray &md5) const = 0;
    virtual KoResource *byFileName(const QString &fileName) const = 0;

private:
    QString m_type;
    QString m_extensions;

protected:

    QMutex m_loadLock;

};

/**
 * KoResourceServer manages the resources of one type. It stores,
 * loads and saves the resources.  To keep track of changes the server
 * can be observed with a KoResourceServerObserver
 *
 * The \p Policy template parameter defines the way how the lifetime

 * of a resource is handled.  There are to predefined policies:

 *
 *   o PointerStroragePolicy --- usual pointers with ownership over
 *                               the resource.

 *   o SharedPointerStroragePolicy --- shared pointers. The server does no
 *                                     extra handling for the lifetime of
 *                                     the resource.
 *
 * Use the former for usual resources and the latter for shared pointer based
 * ones.
 */

template <class T, class Policy = PointerStroragePolicy<T> >
class KoResourceServer : public KoResourceServerBase
{
public:
    typedef typename Policy::PointerType PointerType;
    typedef KoResourceServerObserver<T, Policy> ObserverType;
    KoResourceServer(const QString& type, const QString& extensions)
        : KoResourceServerBase(type, extensions)
    {
        m_blackListFile = KStandardDirs::locateLocal("data", "krita/" + type + ".blacklist");
        m_blackListFileNames = readBlackListFile();
        m_tagStore = new KoResourceTagStore(this);
    }

    virtual ~KoResourceServer()
    {
        if (m_tagStore) {
            delete m_tagStore;
        }

        foreach(ObserverType* observer, m_observers) {
            observer->unsetResourceServer();
        }

        foreach(PointerType res, m_resources) {
            Policy::deleteResource(res);
        }

        m_resources.clear();

    }

    int resourceCount() const {
        return m_resources.size();
    }

    /**
     * Loads a set of resources and adds them to the resource server.
     * If a filename appears twice the resource will only be added once. Resources that can't
     * be loaded or and invalid aren't added to the server.
     * @param filenames list of filenames to be loaded
     */
    void loadResources(QStringList filenames) {
        QStringList uniqueFiles;

        while (!filenames.empty()) {

            QString front = filenames.first();
            filenames.pop_front();

            QString fname = QFileInfo(front).fileName();

            // XXX: Don't load resources with the same filename. Actually, we should look inside
            //      the resource to find out whether they are really the same, but for now this
            //      will prevent the same brush etc. showing up twice.
            if (uniqueFiles.empty() || uniqueFiles.indexOf(fname) == -1) {
                m_loadLock.lock();
                uniqueFiles.append(fname);
                QList<PointerType> resources = createResources(front);
                foreach(PointerType resource, resources) {
                    Q_CHECK_PTR(resource);
                    if (resource->load() && resource->valid() && !resource->md5().isEmpty()) {
                        QByteArray md5 = resource->md5();
                        m_resourcesByMd5[md5] = resource;

                        m_resourcesByFilename[resource->shortFilename()] = resource;

                        if ( resource->name().isEmpty() ) {
                            resource->setName( fname );
                        }
                        if (m_resourcesByName.contains(resource->name())) {
                            resource->setName(resource->name() + "(" + resource->shortFilename() + ")");
                        }
                        m_resourcesByName[resource->name()] = resource;
                        notifyResourceAdded(resource);
                    }
                    else {
                        kWarning() << "Loading resource " << front << "failed";
                        Policy::deleteResource(resource);
                    }
                }
                m_loadLock.unlock();
            }
        }

        m_resources = sortedResources();
        m_tagStore->loadTags();

        foreach(ObserverType* observer, m_observers) {
            observer->syncTaggedResourceView();
        }

        kDebug(30009) << "done loading  resources for type " << type();
    }


    /// Adds an already loaded resource to the server
    bool addResource(PointerType resource, bool save = true, bool infront = false) {
        if (!resource->valid()) {
            kWarning(30009) << "Tried to add an invalid resource!";
            return false;
        }

        if (save) {
            QFileInfo fileInfo(resource->filename());

            if (fileInfo.exists()) {
                QString filename = fileInfo.path() + "/" + fileInfo.baseName() + "XXXXXX" + "." + fileInfo.suffix();
                kDebug() << "fileName is " << filename;
                QTemporaryFile file(filename);
                if (file.open()) {
                    kDebug() << "now " << file.fileName();
                    resource->setFilename(file.fileName());
                }
            }

            if (!resource->save()) {
                kWarning(30009) << "Could not save resource!";
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

        m_resourcesByFilename[resource->shortFilename()] = resource;
        m_resourcesByMd5[resource->md5()] = resource;
        m_resourcesByName[resource->name()] = resource;
        if (infront) {
            m_resources.insert(0, resource);
        }
        else {
            m_resources.append(resource);
        }

        notifyResourceAdded(resource);

        return true;
    }
    
    /*Removes a given resource from the blacklist.
     */
    bool removeFromBlacklist(PointerType resource) {
        if (m_blackListFileNames.contains(resource->filename())) {
            m_blackListFileNames.removeAll(resource->filename());
            writeBlackListFile();
            }
            else{
                kWarning(30009)<<"Doesn't contain filename";
                return false;
            }
        
        
        //then return true//
        return true;
    }
    /// Remove a resource from Resource Server but not from a file
    bool removeResourceFromServer(PointerType resource){
        if ( !m_resourcesByFilename.contains( resource->shortFilename() ) ) {
            return false;
        }
        m_resourcesByMd5.remove(resource->md5());
        m_resourcesByName.remove(resource->name());
        m_resourcesByFilename.remove(resource->shortFilename());
        m_resources.removeAt(m_resources.indexOf(resource));
        m_tagStore->removeResource(resource);
        notifyRemovingResource(resource);

        Policy::deleteResource(resource);
        return true;
    }

    /// Remove a resource from the resourceserver and blacklist it

    bool removeResourceAndBlacklist(PointerType resource) {

        if ( !m_resourcesByFilename.contains( resource->shortFilename() ) ) {
            return false;
        }
        m_resourcesByMd5.remove(resource->md5());
        m_resourcesByName.remove(resource->name());
        m_resourcesByFilename.remove(resource->shortFilename());
        m_resources.removeAt(m_resources.indexOf(resource));
        m_tagStore->removeResource(resource);
        notifyRemovingResource(resource);

        m_blackListFileNames.append(resource->filename());
        writeBlackListFile();
        Policy::deleteResource(resource);
        return true;
    }

    QList<PointerType> resources() {
        m_loadLock.lock();
        QList<PointerType> resourceList = m_resources;
        foreach(PointerType r, m_resourceBlackList) {
            resourceList.removeOne(r);
        }
        m_loadLock.unlock();
        return resourceList;
    }

    /// Returns path where to save user defined and imported resources to
    virtual QString saveLocation() {
        return KGlobal::dirs()->saveLocation(type().toLatin1());
    }

    /**
     * Creates a new resource from a given file and adds them to the resource server
     * The base implementation does only load one resource per file, override to implement collections
     * @param filename file name of the resource file to be imported
     * @param fileCreation decides whether to create the file in the saveLocation() directory
     */
    virtual bool importResourceFile(const QString & filename , bool fileCreation=true) {

        QFileInfo fi(filename);
        if (!fi.exists())
            return false;
        if ( fi.size() == 0)
            return false;

        PointerType resource = createResource( filename );
        resource->load();
        if (!resource->valid()) {
            kWarning(30009) << "Import failed! Resource is not valid";
            Policy::deleteResource(resource);

            return false;

        }

        if (fileCreation) {
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
        }


        if(!addResource(resource)) {
            Policy::deleteResource(resource);
        }

        return true;
    }

    /// Removes the resource file from the resource server
    virtual void removeResourceFile(const QString & filename)
    {
        QFileInfo fi(filename);

        PointerType resource = resourceByFilename(fi.fileName());
        if (!resource) {
            kWarning(30009) << "Resource file do not exist ";
            return;
        }

        if (!removeResourceFromServer(resource))
            return;
    }


    /**
     * Addes an observer to the server
     * @param observer the observer to be added
     * @param notifyLoadedResources determines if the observer should be notified about the already loaded resources
     */
    void addObserver(ObserverType* observer, bool notifyLoadedResources = true)
    {
        m_loadLock.lock();
        if(observer && !m_observers.contains(observer)) {
            m_observers.append(observer);

            if(notifyLoadedResources) {
                foreach(PointerType resource, m_resourcesByFilename) {
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
    void removeObserver(ObserverType* observer)
    {
        int index = m_observers.indexOf( observer );
        if( index < 0 )
            return;

        m_observers.removeAt( index );
    }

    PointerType resourceByFilename(const QString& filename) const
    {
        if (m_resourcesByFilename.contains(filename)) {
            return m_resourcesByFilename[filename];
        }
        return 0;
    }


    PointerType resourceByName( const QString& name ) const
    {
        if (m_resourcesByName.contains(name)) {
            return m_resourcesByName[name];
        }
        return 0;
    }

    PointerType resourceByMD5(const QByteArray& md5) const
    {
        return m_resourcesByMd5.value(md5);
    }

    /**
     * Call after changing the content of a resource;
     * Notifies the connected views.
     */
    void updateResource( PointerType resource )
    {
        notifyResourceChanged(resource);
    }

    QStringList blackListedFiles() const
    {
        return m_blackListFileNames;
    }

    void removeBlackListedFiles() {
        QStringList remainingFiles; // Files that can't be removed e.g. no rights will stay blacklisted
        foreach(const QString &filename, m_blackListFileNames) {
            QFile file( filename );
            if( ! file.remove() ) {
                remainingFiles.append(filename);
            }
        }
        m_blackListFileNames = remainingFiles;
        writeBlackListFile();
    }

    QStringList tagNamesList() const
    {
        return m_tagStore->tagNamesList();
    }

    // don't use these method directly since it doesn't update views!
    void addTag( KoResource* resource,const QString& tag)
    {
        m_tagStore->addTag(resource,tag);
    }

    // don't use these method directly since it doesn't update views!
    void delTag( KoResource* resource,const QString& tag)
    {
        m_tagStore->delTag(resource,tag);
    }

    QStringList searchTag(const QString& lineEditText)
    {
        return m_tagStore->searchTag(lineEditText);
    }

    void tagCategoryAdded(const QString& tag)
    {
        m_tagStore->serializeTags();
        foreach(ObserverType* observer, m_observers) {
            observer->syncTagAddition(tag);
        }
    }

    void tagCategoryRemoved(const QString& tag)
    {
        m_tagStore->delTag(tag);
        m_tagStore->serializeTags();
        foreach(ObserverType* observer, m_observers) {
            observer->syncTagRemoval(tag);
        }
    }

    void tagCategoryMembersChanged()
    {
        m_tagStore->serializeTags();
        foreach(ObserverType* observer, m_observers) {
            observer->syncTaggedResourceView();
        }
    }

    QStringList queryResources(const QString &query) const
    {
        return m_tagStore->searchTag(query);
    }

    QStringList assignedTagsList(KoResource* resource) const
    {
        return m_tagStore->assignedTagsList(resource);
    }


    /**
     * Create one or more resources from a single file. By default one resource is created.
     * Overide to create more resources from the file.
     * @param filename the filename of the resource or resource collection
     */
    virtual QList<PointerType> createResources( const QString & filename )
    {
        QList<PointerType> createdResources;
        createdResources.append(createResource(filename));
        return createdResources;
    }

    virtual PointerType createResource( const QString & filename ) = 0;

    /// Return the currently stored resources in alphabetical order, overwrite for customized sorting
    virtual QList<PointerType> sortedResources()
    {
        QMap<QString, PointerType> sortedNames;
        foreach(const QString &name, m_resourcesByName.keys()) {
            sortedNames.insert(name.toLower(), m_resourcesByName[name]);
        }
        return sortedNames.values();
    }

protected:

    void notifyResourceAdded(PointerType resource)
    {
        foreach(ObserverType* observer, m_observers) {
            observer->resourceAdded(resource);
        }
    }

    void notifyRemovingResource(PointerType resource)
    {
        foreach(ObserverType* observer, m_observers) {
            observer->removingResource(resource);
        }
    }

    void notifyResourceChanged(PointerType resource)
    {
        foreach(ObserverType* observer, m_observers) {
            observer->resourceChanged(resource);
        }
    }

    /// Reads the xml file and returns the filenames as a list
    QStringList readBlackListFile()
    {
        QStringList filenameList;

        QFile f(m_blackListFile);
        if (!f.open(QIODevice::ReadOnly)) {
            return filenameList;
        }

        QDomDocument doc;
        if (!doc.setContent(&f)) {
            kWarning() << "The file could not be parsed.";
            return filenameList;
        }

        QDomElement root = doc.documentElement();
        if (root.tagName() != "resourceFilesList") {
            kWarning() << "The file doesn't seem to be of interest.";
            return filenameList;
        }

        QDomElement file = root.firstChildElement("file");

        while (!file.isNull()) {
            QDomNode n = file.firstChild();
            QDomElement e = n.toElement();
            if (e.tagName() == "name") {
                filenameList.append((e.text()).replace(QString("~"),QDir::homePath()));
            }
            file = file.nextSiblingElement("file");
        }
        return filenameList;
    }

    /// write the blacklist file entries to an xml file
    void writeBlackListFile()
    {
        QFile f(m_blackListFile);

        if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            kWarning() << "Cannot write meta information to '" << m_blackListFile << "'." << endl;
            return;
        }

        QDomDocument doc;
        QDomElement root;

        QDomDocument docTemp("m_blackListFile");
        doc = docTemp;
        doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
        root = doc.createElement("resourceFilesList");
        doc.appendChild(root);
        
        foreach(QString filename, m_blackListFileNames) {
            QDomElement fileEl = doc.createElement("file");
            QDomElement nameEl = doc.createElement("name");
            QDomText nameText = doc.createTextNode(filename.replace(QDir::homePath(),QString("~")));
            nameEl.appendChild(nameText);
            fileEl.appendChild(nameEl);
            root.appendChild(fileEl);
        }
        
        

        QTextStream metastream(&f);
        metastream << doc.toByteArray();
        f.close();
    }

protected:

    KoResource* byMd5(const QByteArray &md5) const
    {
        return Policy::toResourcePointer(resourceByMD5(md5));
    }

    KoResource* byFileName(const QString &fileName) const
    {
        return Policy::toResourcePointer(resourceByFilename(fileName));
    }

private:

    QHash<QString, PointerType> m_resourcesByName;
    QHash<QString, PointerType> m_resourcesByFilename;
    QHash<QByteArray, PointerType> m_resourcesByMd5;

    QList<PointerType> m_resourceBlackList;
    QList<PointerType> m_resources; ///< list of resources in order of addition
    QList<ObserverType*> m_observers;
    QString m_blackListFile;
    QStringList m_blackListFileNames;
    KoResourceTagStore* m_tagStore;

};

template <class T, class Policy = PointerStroragePolicy<T> >
    class KoResourceServerSimpleConstruction : public KoResourceServer<T, Policy>
{
public:
    KoResourceServerSimpleConstruction(const QString& type, const QString& extensions)
: KoResourceServer<T, Policy>(type, extensions)
    {
    }

typename KoResourceServer<T, Policy>::PointerType createResource( const QString & filename ) {
        return new T(filename);
    }
};

#endif // KORESOURCESERVER_H
