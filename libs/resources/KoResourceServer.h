/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>

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

#include "KoResource.h"

#include "koresource_export.h"

#include <KDebug>

class KoResource;

class KORESOURCES_EXPORT KoResourceServerBase {

public:
    KoResourceServerBase() {}
    virtual ~KoResourceServerBase() {}

    virtual void loadResources(QStringList filenames) = 0;
};

template <class T> class KoResourceServer : public KoResourceServerBase {

public:
    KoResourceServer() : m_loaded(false) {}
    virtual ~KoResourceServer() {}

    void loadResources(QStringList filenames) {
        QStringList uniqueFiles;

        while (!filenames.empty())
        {
            QString front = filenames.first();
            filenames.pop_front();

            QString fname = QFileInfo(front).fileName();
            //ebug() << "Loading " << fname << "\n";
            // XXX: Don't load resources with the same filename. Actually, we should look inside
            //      the resource to find out whether they are really the same, but for now this
            //      will prevent the same brush etc. showing up twice.
            if (uniqueFiles.empty() || uniqueFiles.indexOf(fname) == -1) {
                uniqueFiles.append(fname);
                T* resource = createResource(front);
                if (resource->load() && resource->valid())
                {
                    m_resources.append(resource);
                    Q_CHECK_PTR(resource);
                }
                else {
                    delete resource;
                }
            }
        }
        m_loaded = true;
}


    /// Adds an already loaded resource to the server
    void addResource(T* resource) {
        if (!resource->valid()) {
            kWarning(30009) << "Tried to add an invalid resource!";
            return;
        }
        resource->save();

        m_resources.append(resource);
    }

    /// Remove a resource from resourceserver and hard disk
    void removeResource(T* resource) {
        int index = m_resources.indexOf( resource );
        if( index < 0 )
            return;

        QFile file( resource->filename() );

        if( file.remove() )
        {
            m_resources.removeAt( index );
            delete resource;
        }
    }

    QList<T*> resources() {
        if(!m_loaded) {
            return QList<T*>();
        }
        return m_resources;
    }

protected:
    virtual T* createResource( const QString & filename ) { return new T(filename); }

private:
    QList<T*> m_resources;

    bool m_loaded;

};

#endif // KORESOURCESERVER_H
