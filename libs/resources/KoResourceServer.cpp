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

#include "KoResourceServer.h"

#include <QFileInfo>
#include <QStringList>
#include <QThread>
#include <QDir>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include "KoResource.h"
#include "KoPattern.h"
#include "KoColorSet.h"

KoResourceServerBase::KoResourceServerBase(const QString & type)
    : m_type(type), m_loaded(false)
{
}

KoResourceServerBase::~KoResourceServerBase()
{
}

void KoResourceServerBase::loadResources(QStringList filenames)
{
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
            KoResource *resource;
            resource = createResource(front);
            if (resource->load() && resource->valid())
            {
                m_resources.append(resource);
                Q_CHECK_PTR(resource);
                emit resourceAdded(resource);
            }
            else {
                delete resource;
            }
        }
    }
    m_loaded = true;
}

QList<KoResource*> KoResourceServerBase::resources()
{
    if(!m_loaded) {
        return QList<KoResource*>();
    }

    return m_resources;
}

void KoResourceServerBase::addResource(KoResource* resource)
{
    if (!resource->valid()) {
        kWarning(41001) << "Tried to add an invalid resource!";
        return;
    }
    resource->save();

    m_resources.append(resource);
    emit resourceAdded(resource);
}

void KoResourceServerBase::removeResource(KoResource* resource)
{
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

#include "KoResourceServer.moc"

