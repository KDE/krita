/* This file is part of the KDE project
   Copyright (C) 2004 Cedric Pasteur <cedric.pasteur@free.fr>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "test.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char description[] = "A test application for the KoProperty library";

static const char version[] = "0.2";

static KCmdLineOptions options[] =
{
    { "flat", "Flat display: don't display groups\n(useful for testing)", 0 },
    { "ro", "Set all properties as read-only:\n(useful for testing read-only mode)", 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("proptest", "KoProperty Test", version, description,
                     KAboutData::License_GPL, "(C) 2005 Cedric Pasteur", 0, 0, "cedric.pasteur@free.fr");
     about.addAuthor( "Cedric Pasteur", 0, "cedric.pasteur@free.fr" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;
    Test *mainWin = 0;

    if (app.isSessionRestored())
    {
        RESTORE(Test);
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        /// @todo do something with the command line args here

        mainWin = new Test();
        app.setMainWidget( mainWin );
        mainWin->show();

        args->clear();
    }

    // mainWin has WDestructiveClose flag by default, so it will delete itself.
    return app.exec();
}

