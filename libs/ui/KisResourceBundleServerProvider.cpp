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

#include "KisResourceBundleServerProvider.h"
#include "KisResourceServerProvider.h"

#include <QDir>
#include <QApplication>
#include <QGlobalStatic>

#include <kis_debug.h>

#include <KoResourcePaths.h>
#include <KoResource.h>
#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>

Q_GLOBAL_STATIC(KisResourceBundleServerProvider, s_instance)

KisResourceBundleServerProvider::KisResourceBundleServerProvider()
{
    m_resourceBundleServer = new KoResourceServerSimpleConstruction<KisResourceBundle>("kis_resourcebundles", "*.bundle");
    QStringList files = KoResourceServerProvider::blacklistFileNames(m_resourceBundleServer->fileNames(), m_resourceBundleServer->blackListedFiles());
//    qDebug() << "Bundle files to load" << files;
    m_resourceBundleServer->loadResources(files);

    Q_FOREACH (KisResourceBundleSP bundle, m_resourceBundleServer->resources()) {
        if (!bundle->install()) {
            warnKrita << "Could not install resources for bundle" << bundle->name();
        }
    }
}

KisResourceBundleServerProvider::~KisResourceBundleServerProvider()
{
    delete m_resourceBundleServer;
}

KisResourceBundleServerProvider* KisResourceBundleServerProvider::instance()
{
    return s_instance;
}

KoResourceServer<KisResourceBundle> *KisResourceBundleServerProvider::resourceBundleServer()
{
    return m_resourceBundleServer;
}

