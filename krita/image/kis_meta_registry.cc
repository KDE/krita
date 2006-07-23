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
#include <kglobal.h>

#include <config.h>

#include "KoColorSpaceRegistry.h"
#include "kis_math_toolbox.h"
#include "kis_meta_registry.h"

KisMetaRegistry * KisMetaRegistry::m_singleton = 0;

KisMetaRegistry::KisMetaRegistry()
{
    m_csRegistry = KoColorSpaceRegistry::instance();
    m_mtRegistry = new KisMathToolboxFactoryRegistry();
}

KisMetaRegistry::~KisMetaRegistry()
{
}

KisMetaRegistry * KisMetaRegistry::instance()
{
    if ( KisMetaRegistry::m_singleton == 0 ) {
        KisMetaRegistry::m_singleton = new KisMetaRegistry();
    }
    return KisMetaRegistry::m_singleton;
}

