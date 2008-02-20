/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <KoPluginLoader.h>

#include "kis_brush_registry.h"

KisBrushRegistry * KisBrushRegistry::m_singleton = 0;

KisBrushRegistry::KisBrushRegistry()
{
    Q_ASSERT(KisBrushRegistry::m_singleton == 0);
}

KisBrushRegistry::~KisBrushRegistry()
{
}

KisBrushRegistry* KisBrushRegistry::instance()
{
    if(KisBrushRegistry::m_singleton == 0)
    {
        KisBrushRegistry::m_singleton = new KisBrushRegistry();
        KoPluginLoader::instance()->load("Krita/Brush", "(Type == 'Service') and ([X-Krita-Version] == 3)");
        Q_CHECK_PTR(KisBrushRegistry::m_singleton);
    }
    return KisBrushRegistry::m_singleton;
}


KisBrushFactorySP KisBrushRegistry::factory(const QString & extension) const
{
    if (m_fileBrushes.contains(extension)) {
        return m_fileBrushes[extension];
    }
    else {
        return 0;
    }
}

void KisBrushRegistry::registerFactory(KisBrushFactorySP factory, const QString & extension)
{
    m_fileBrushes[extension] = factory;
}

QList<KisBrushFactorySP> KisBrushRegistry::dynamicBrushFactories() const
{
    return m_dynamicBrushes.values();
}

KisBrushFactorySP KisBrushRegistry::dynamicBrushFactory( const QString & id) const
{
    if (m_dynamicBrushes.contains(id)) {
        return m_dynamicBrushes[id];
    }
    else {
        return 0;
    }
}

void KisBrushRegistry::registerDynamicFactory(KisBrushFactorySP factory, const QString & id)
{
    m_dynamicBrushes[id] = factory;
}
    