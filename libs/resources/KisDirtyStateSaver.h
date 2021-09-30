/*
 * SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef KISDIRTYSTATESAVER_H
#define KISDIRTYSTATESAVER_H

#include <KoResource.h>

#include "kritaresources_export.h"
/**
 * Never use manual save/restore calls to
 * KoResource::isDirty()/KoResource::setDirty()! They will lead to
 * hard-to-tack-down bugs when the dirty state will not be
 * restored on jumps like 'return', 'break' or exception.
 */
template <typename T>
class KisDirtyStateSaver
{
public:
    KisDirtyStateSaver(T resource)
        : m_resource(resource)
        , m_isDirty(resource->isDirty())

    {
    }

    ~KisDirtyStateSaver() {
        if (m_resource) {
            m_resource->setDirty(m_isDirty);
        }
    }

private:
    T m_resource;
    bool m_isDirty = false;
};

#endif // KISDIRTYSTATESAVER_H
