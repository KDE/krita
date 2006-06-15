/*
 *  main.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include <kcmdlineargs.h>
#include <KoApplication.h>
#include <krita_export.h>

#include "ui/kis_aboutdata.h"

static const KCmdLineOptions options[] = {
    { "+[file(s)]", I18N_NOOP("File(s) or URL(s) to open"), 0 },
    KCmdLineLastOption
};

extern "C" KRITA_EXPORT int kdemain(int argc, char **argv)
{
    KCmdLineArgs::init(argc, argv, newKritaAboutData());
    KCmdLineArgs::addCmdLineOptions(options);

    KoApplication app;

    if (!app.start())
        return 1;

    return app.exec();
}

