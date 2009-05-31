/*
 * Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "ExtensionsManager.h"

#include <QApplication>
#include <kurl.h>
#include <kio/netaccess.h>
#include <QFile>

ExtensionsManager* ExtensionsManager::s_instance = 0;

ExtensionsManager::ExtensionsManager() {
}

ExtensionsManager::~ExtensionsManager() {
}

ExtensionsManager* ExtensionsManager::instance() {
    if(!s_instance) {
        s_instance = new ExtensionsManager;
    }
    return s_instance;
}

QList<Extension*> ExtensionsManager::installedExtension() {
  return m_installedExtension;
}

bool ExtensionsManager::installExtension(const KUrl& uri) {
    if (!KIO::NetAccess::exists(uri, KIO::NetAccess::SourceSide, qApp->activeWindow())) {
      return false;
    }

    // We're not set up to handle asynchronous loading at the moment.
    QString tmpFile;

    if (KIO::NetAccess::download(uri, tmpFile, qApp->activeWindow())) {
        KUrl uriTF;
        uriTF.setPath(tmpFile);

        // open the file
        bool result = false;
        QFile *fp = new QFile(uriTF.path());
        if (fp->exists()) {
            result =  installExtension(fp);
        }
        delete fp;
        KIO::NetAccess::removeTempFile(tmpFile);
        return result;
    }
    return false;
}

bool ExtensionsManager::installExtension(QIODevice* _device) {
    return false;
}
