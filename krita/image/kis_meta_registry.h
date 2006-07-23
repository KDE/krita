/*
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
#ifndef _KIS_META_REGISTRY_
#define _KIS_META_REGISTRY_

#include <krita_export.h>

class KoColorSpaceRegistry;
class KisMathToolboxFactoryRegistry;

/**
 * A single singleton that provides access to several registries.
 *
 * XXX: Maybe this should go into the SDK
 */
class KRITAIMAGE_EXPORT KisMetaRegistry {

public:

    virtual ~KisMetaRegistry();
    static KisMetaRegistry* instance();

    KoColorSpaceRegistry * csRegistry() { return m_csRegistry; };
    KisMathToolboxFactoryRegistry* mtRegistry() { return m_mtRegistry; };
private:

    KisMetaRegistry();
    KisMetaRegistry( const KisMetaRegistry& );
    KisMetaRegistry operator=( const KisMetaRegistry& );

    static KisMetaRegistry * m_singleton;

    KoColorSpaceRegistry * m_csRegistry;
    KisMathToolboxFactoryRegistry* m_mtRegistry;
};
#endif
