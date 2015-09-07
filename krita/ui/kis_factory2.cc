/*
 *  kis_factory.cc - part of Krayon
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
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

//#ifdef _MSC_VER // this removes KDEWIN extensions to stdint.h: required by exiv2
//#define KDEWIN_STDINT_H
//#endif

#include "kis_factory2.h"

#include <QStringList>
#include <QDir>

#include <kcomponentdata.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kglobal.h>

#include <kis_debug.h>

#include "kis_aboutdata.h"

#include "KisPart.h"


K4AboutData* KisFactory::s_aboutData = 0;
KComponentData* KisFactory::s_componentData = 0;

KisFactory::KisFactory()
{
    (void)componentData();
}

KisFactory::~KisFactory()
{
    delete s_aboutData;
    s_aboutData = 0;
    delete s_componentData;
    s_componentData = 0;
}


K4AboutData* KisFactory::aboutData()
{
    if (!s_aboutData) {
        s_aboutData = newKritaAboutData();
    }
    return s_aboutData;
}

const KComponentData &KisFactory::componentData()
{
    if (!s_componentData) {
        s_componentData = new KComponentData(aboutData());
        Q_CHECK_PTR(s_componentData);

        // for cursors
        KGlobal::dirs()->addResourceType("kis_pics", "data", "krita/pics/");

        // for images in the paintop box
        KGlobal::dirs()->addResourceType("kis_images", "data", "krita/images/");

        KGlobal::dirs()->addResourceType("icc_profiles", "data", "krita/profiles/");

        // Tell the iconloader about share/apps/calligra/icons
        KIconLoader::global()->addAppDir("calligra");
    }

    return *s_componentData;
}

