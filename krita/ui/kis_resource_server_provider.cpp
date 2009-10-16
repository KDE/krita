/*
 *  kis_resourceserver.cc - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <elter@kde.org>
 *  Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_resource_server_provider.h"

#include <QDir>

#include <kglobal.h>
#include <kstandarddirs.h>
#include <kcomponentdata.h>

#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

#include <kis_debug.h>
#include <kis_pattern.h>
#include <kis_paintop_preset.h>

KisResourceServerProvider::KisResourceServerProvider()
{
    KGlobal::mainComponent().dirs()->addResourceType("kis_patterns", "data", "krita/patterns/");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_patterns", "/usr/share/create/patterns/gimp");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_patterns", QDir::homePath() + QString("/.create/patterns/gimp"));

    KGlobal::mainComponent().dirs()->addResourceType("kis_paintoppresets", "data", "krita/paintoppresets/");
    KGlobal::mainComponent().dirs()->addResourceDir("kis_paintoppresets", QDir::homePath() + QString("/.create/paintoppresets/krita"));


    m_patternServer = new KoResourceServer<KisPattern>("kis_patterns", "*.jpg:*.gif:*.png:*.tif:*.xpm:*.bmp:*.pat");
    patternThread = new KoResourceLoaderThread(m_patternServer);
    connect(patternThread, SIGNAL(finished()), this, SLOT(patternThreadDone()));
    patternThread->start();

    m_paintOpPresetServer = new KoResourceServer<KisPaintOpPreset>("kis_paintoppresets", "*.kpp");
    paintOpPresetThread = new KoResourceLoaderThread(m_paintOpPresetServer);
    connect(paintOpPresetThread, SIGNAL(finished()), this, SLOT(paintOpPresetThreadDone()));
    paintOpPresetThread->start();

}

KisResourceServerProvider::~KisResourceServerProvider()
{
    dbgRegistry << "deleting KisResourceServerProvider";
}

KisResourceServerProvider* KisResourceServerProvider::instance()
{
    K_GLOBAL_STATIC(KisResourceServerProvider, s_instance);
    return s_instance;
}


KoResourceServer<KisPattern>* KisResourceServerProvider::patternServer()
{
    return m_patternServer;
}

KoResourceServer<KisPaintOpPreset>* KisResourceServerProvider::paintOpPresetServer()
{
    return m_paintOpPresetServer;
}

void KisResourceServerProvider::patternThreadDone()
{
    delete patternThread;
    patternThread = 0;
}

void KisResourceServerProvider::paintOpPresetThreadDone()
{
    delete paintOpPresetThread;
    paintOpPresetThread = 0;
}

#include "kis_resource_server_provider.moc"
