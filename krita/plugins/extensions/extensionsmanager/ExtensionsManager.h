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
#ifndef EXTENSIONS_MANAGER
#define EXTENSIONS_MANAGER

#include <QList>

class QIODevice;
class KUrl;
class Extension;

/**
 * XXX
 */
class ExtensionsManager
{

    ExtensionsManager();

    virtual ~ExtensionsManager();

public:

    static ExtensionsManager* instance();

    QList<Extension*> installedExtension();

    bool installExtension(const KUrl& _file);

    bool installExtension(QIODevice* _device);

private:
    QList<Extension*> m_installedExtension;
};

#endif
