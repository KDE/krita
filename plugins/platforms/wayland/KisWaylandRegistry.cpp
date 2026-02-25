/*
 *  SPDX-FileCopyrightText: 2026 Luna Lovecraft <ciubix8514@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisWaylandRegistry.h"
#include <qguiapplication.h>

Q_GLOBAL_STATIC(KisWaylandRegistry, s_waylandRegistry)

struct KisWaylandRegistry::Private
{
    QHash<uint32_t, std::pair<QString, uint32_t>> globals;
    bool initialized = false;
};

KisWaylandRegistry::KisWaylandRegistry()
    : d(new Private)
{
}

KisWaylandRegistry::~KisWaylandRegistry()
{
}

void KisWaylandRegistry::registry_global(uint32_t name, const QString &interface, uint32_t version)
{
    d->globals.insert(name, std::pair<QString, uint32_t>(interface, version));
}

bool KisWaylandRegistry::globalExists(const QString& interface)
{
    //Check if the interface is explicitly disabled
    static QStringList interfaceBlacklist = qEnvironmentVariable("QT_WAYLAND_DISABLED_INTERFACES").split(u',');

    if (interfaceBlacklist.contains(interface)) {
        return false;
    }

    Q_FOREACH(auto global, d->globals.values()) {
        if (global.first == interface)
            return true;
    }

    return false;
}

void KisWaylandRegistry::registry_global_remove(uint32_t name)
{
    d->globals.remove(name);
}

KisWaylandRegistry* KisWaylandRegistry::getOrCreate()
{
    if (!s_waylandRegistry->d->initialized) {
        KisWaylandRegistry *registry = s_waylandRegistry;
        registry->d->initialized = true;

        auto waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
        auto display = waylandApp->display();
        auto regisry = ::wl_display_get_registry(display);
        registry->init(regisry);

        //Wait for the compositor to populate the registry
        ::wl_display_roundtrip(display);
    }

    return s_waylandRegistry;
}
