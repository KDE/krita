/*
 *  main.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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
 */

#include <dcopclient.h>

#include <klocale.h>
#include <kcmdlineargs.h>
#include <koApplication.h>

#include "core/kis_aboutdata.h"


static const KCmdLineOptions options[] =
{
	{ "+[file(s)]", I18N_NOOP( "File(s) or URL(s) to Open" ), 0 },
	{ 0, 0, 0 }
};

int main( int argc, char **argv )
{
    KCmdLineArgs::init( argc, argv, newKrayonAboutData() );
    KCmdLineArgs::addCmdLineOptions( options );

    KoApplication app;

    app.dcopClient()->attach();
    app.dcopClient()->registerAs( "krayon" );

    if (!app.start())
        return 1;
    app.exec();
    return 0;
}
