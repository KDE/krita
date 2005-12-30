/*
 * This file is part of the KDE project
 *
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include <kis_doc.h>
#include <kis_global.h>
#include <kis_types.h>
#include <kis_view.h>

#include "historydocker.h"

typedef KGenericFactory<KritaHistoryDocker> KritaHistoryDockerFactory;
K_EXPORT_COMPONENT_FACTORY( kritahistorydocker, KritaHistoryDockerFactory( "krita" ) )

KritaHistoryDocker::KritaHistoryDocker(QObject *parent, const char *name, const QStringList &)
        : KParts::Plugin(parent, name)
{
    


    if ( parent->inherits("KisView") )
    {
        setInstance(KritaHistoryDockerFactory::instance());
        // Create history docker
        // Add the docker to the docker manager
        // Connect the undo system to the docker
    }

}

KritaHistoryDocker::~KritaHistoryDocker()
{
}
