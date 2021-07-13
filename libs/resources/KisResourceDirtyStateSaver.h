/*
 * SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
    KisResourceDirtyStateSaver(KoResourceSP resource);

    /// Extra constructor to be called from KoResource itself
    KisResourceDirtyStateSaver(KoResource* resource);

    ~KisResourceDirtyStateSaver();

private:

    KoResourceSP m_resource;
    KoResource *m_parentResource {0};
    bool m_isDirty;

};

#endif // KISRESOURCEDIRTYSTATESAVER_H
