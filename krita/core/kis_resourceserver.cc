/*
 *  kis_resourceserver.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Sven Langkamp <longamp@reallygood.de>
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
#include <qstringlist.h>
#include <qfileinfo.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kinstance.h>

#include "kis_factory.h"
#include "kis_resource.h"

#include "kis_resourceserver.h"

KisResourceServerBase::KisResourceServerBase(QString type, QStringList fileExtensions)
    : m_fileExtensions(fileExtensions), m_type(type), m_loaded(false)
{
}

KisResourceServerBase::~KisResourceServerBase()
{
}

void KisResourceServerBase::loadResources()
{
    QStringList filenames;
    QStringList uniqueFiles;
    
    QStringList::Iterator it;
    for ( it = m_fileExtensions.begin(); it != m_fileExtensions.end(); ++it ) 
        filenames += KisFactory::instance() -> dirs() -> findAllResources(m_type.ascii(), (*it));
    
    while( !filenames.empty() )
    {
        
        QString front = *filenames.begin();
        filenames.pop_front();
        
        QString fname = QFileInfo(front).fileName();
        
        // XXX: Don't load resources with the same filename. Actually, we should look inside
        //      the resource to find out whether they are really the same, but for now this
        //      will prevent the same brush etc. showing up twice.
        if (uniqueFiles.empty() || uniqueFiles.find(fname) == uniqueFiles.end()) {
            uniqueFiles.append(fname);
            KisResource *resource;
            resource = createResource(front);
            if(resource -> load() && resource -> valid())
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

QValueList<KisResource*> KisResourceServerBase::resources()
{
    if(!m_loaded)
        loadResources();

    return m_resources;
}
