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

#include <kurl.h>

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

void ExtensionsManager::installExtension(const KUrl& _file) {
  
}
