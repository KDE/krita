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

#include <koApplication.h>
#include <koDocument.h>
#include <koMainWindow.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <dcopclient.h>
#include "krita_aboutdata.h"


static const KCmdLineOptions options[]=
{
	{"+[file]", I18N_NOOP("File to open"),0},
	KCmdLineLastOption
};

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, newkritaAboutData() );
    KCmdLineArgs::addCmdLineOptions( options );
    KoApplication app;

    app.dcopClient()->attach();
    app.dcopClient()->registerAs( "krita" );

    if (!app.start()) // parses command line args, create initial docs and shells
	return 1;
    return app.exec();
}
