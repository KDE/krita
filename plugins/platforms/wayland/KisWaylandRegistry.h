/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef _KIS_WAYLAND_REGISTRY_
#define _KIS_WAYLAND_REGISTRY_
#include "qwayland-wayland.h"
#include <qscopedpointer.h>

/// Wayland registry for tracking wayland globals
class KisWaylandRegistry : QtWayland::wl_registry
{
public:
    static KisWaylandRegistry *getOrCreate();
    bool globalExists(const QString &interface);
    KisWaylandRegistry();
    ~KisWaylandRegistry();

protected:
    void registry_global(uint32_t name, const QString &interface, uint32_t version) override;
    void registry_global_remove(uint32_t name) override;

private:
    struct Private;
    QScopedPointer<Private> d;
};

#endif
