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
#ifndef KIS_BRUSH_REGISTRY_H
#define KIS_BRUSH_REGISTRY_H

#include <QObject>

#include "kis_brush.h"
#include "kis_types.h"
#include "krita_export.h"

/**
 * KisBrushRegistry keeps track of all the various types of brushes.
 * Brushes are organized on file type, if loadable from a file. We
 * don't do mimetypes for brush files (yet).
 *
 * There are also dynamic brushes (text and custom spring to mind).
 * Those can be retrieved by ID or it is possible to get a list of
 * factories for those types.
 */
class KisBrushRegistry {

public:
    
    ~KisBrushRegistry();

public:
    
    static KisBrushRegistry* instance();

    /**
     * Get the factory for the specified file extension
     */
    KisBrushFactorySP factory(const QString & extension) const;

    /**
     * register the factory for the specified file extension.
     */
    void registerFactory(KisBrushFactorySP factory, const QString & extension);

    /**
     * Get a list of factories that create dynamic brushes.
     */
    QList<KisBrushFactorySP> dynamicBrushFactories() const;

    /**
     * Get a dynamic brush factory for the give ID
     */
    KisBrushFactorySP dynamicBrushFactory( const QString & id) const;

    /**
     * Register a dynamic brush factory
     */
    void registerDynamicFactory(KisBrushFactorySP factory, const QString & id);
    

private:

    KisBrushRegistry();
    KisBrushRegistry(const KisBrushRegistry&);
    KisBrushRegistry operator=(const KisBrushRegistry&);

private:

    static KisBrushRegistry *m_singleton;
    
    QHash<QString, KisBrushFactorySP> m_fileBrushes;

    QHash<QString, KisBrushFactorySP> m_dynamicBrushes;
};

#endif
