/***************************************************************************
 *   Copyright (C) 2004 by Boudewijn Rempt                                 *
 *   boud@valdyas.org                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

/* This template is based off of the KOffice example written by Torben Weis <weis@kde.org
   It was converted to a KDevelop template by Ian Reinhart Geiser <geiseri@yahoo.com>
*/
#include <krita_factory.h>
#include <krita_part.h>
#include <krita_aboutdata.h>
#include <kinstance.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>

extern "C"
{
    void* init_libkritapart()
    {
        KGlobal::locale()->insertCatalogue("krita");
        return new kritaFactory;
    }
};

KInstance* kritaFactory::s_global = 0L;
KAboutData* kritaFactory::s_aboutData = 0L;

kritaFactory::kritaFactory( QObject* parent, const char* name )
    : KoFactory( parent, name )
{
    global();
}

kritaFactory::~kritaFactory()
{
    delete s_aboutData;
    s_aboutData = 0L;
    delete s_global;
    s_global = 0L;
}

KParts::Part* kritaFactory::createPartObject( QWidget *parentWidget, const char *widgetName, QObject* parent, const char* name, const char* classname, const QStringList & )
{
    // If classname is "KoDocument", our host is a koffice application
    // otherwise, the host wants us as a simple part, so switch to readonly and single view.
    bool bWantKoDocument = ( strcmp( classname, "KoDocument" ) == 0 );

    // parentWidget and widgetName are used by KoDocument for the "readonly+singleView" case.
    kritaPart *part = new kritaPart( parentWidget, widgetName, parent, name, !bWantKoDocument );

    if ( !bWantKoDocument )
      part->setReadWrite( false );

    return part;
}

KAboutData* kritaFactory::aboutData()
{
    if ( !s_aboutData )
        // Change this, of course
        s_aboutData = newkritaAboutData();
    return s_aboutData;
}

KInstance* kritaFactory::global()
{
    if ( !s_global )
    {
        s_global = new KInstance( aboutData() );
        // Add any application-specific resource directories here

        // Tell the iconloader about share/apps/koffice/icons
        s_global->iconLoader()->addAppDir("koffice");
    }
    return s_global;
}

#include <krita_factory.moc>
