/*
 * Copyright (C) 2019 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KISRESOURCEDIRTYSTATESAVER_H
#define KISRESOURCEDIRTYSTATESAVER_H

#include <KoResource.h>

#include "kritaresources_export.h"
/**
 * Never use manual save/restore calls to
 * KoResource::isDirty()/KoResource::setDirty()! They will lead to
 * hard-to-tack-down bugs when the dirty state will not be
 * restored on jumps like 'return', 'break' or exception.
 */
class KRITARESOURCES_EXPORT KisResourceDirtyStateSaver
{
public:
    KisResourceDirtyStateSaver(KoResourceSP resource)
        : m_resource(resource)
        , m_isDirty(resource->isDirty())

    {
    }

    /// Extra constructor to be called from KoResource itself
    KisResourceDirtyStateSaver(KoResource *resource)
        : m_resource(resource)
        , m_isDirty(resource->isDirty())
    {
    }

    ~KisResourceDirtyStateSaver() {
        if (m_resource) {
            m_resource->setDirty(m_isDirty);
        }
        else if (m_parentResource) {
            m_parentResource->setDirty(m_isDirty);
        }
    }

private:

    KoResourceSP m_resource;
    KoResource *m_parentResource {0};
    bool m_isDirty;

};

#endif // KISRESOURCEDIRTYSTATESAVER_H
