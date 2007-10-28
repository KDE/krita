/*  This file is part of the KDE project

    Copyright (c) 1999 Matthias Elter <elter@kde.org>
    Copyright (c) 2003 Patrick Julien <freak@codepimps.org>
    Copyright (c) 2005 Sven Langkamp <sven.langkamp@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef KORESOURCESERVERREGISTRY_H
#define KORESOURCESERVERREGISTRY_H

#include "KoGenericRegistry.h"
#include <koresource_export.h>

#include "KoResourceServer.h"

class KORESOURCES_EXPORT KoResourceServerRegistry : public KoGenericRegistry<KoResourceServerBase*>
{
public:
    virtual ~KoResourceServerRegistry();

    static KoResourceServerRegistry* instance();

private:
    KoResourceServerRegistry();
    KoResourceServerRegistry(const KoResourceServerRegistry&);
    KoResourceServerRegistry operator=(const KoResourceServerRegistry&);

    QStringList getFileNames( const QString & extensions, const QString & type );

    static KoResourceServerRegistry *m_singleton;
};

#endif // KORESOURCESERVERREGISTRY_H
