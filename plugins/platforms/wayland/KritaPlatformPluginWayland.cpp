/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>

#include <kis_assert.h>

#include <KisExtendedModifiersMapperWayland.h>
#include <waylandcolormanagement/KisWaylandSurfaceColorManager.h>
#include <waylandcolormanagement/KisWaylandOutputColorInfo.h>

#include <QWindow>

namespace detail {

// just a simple wrapper that unpacks arguments from a QVariantList into
// a proper interface for KisWaylandSurfaceColorManager
class KisWaylandSurfaceColorManagerWrapper: public KisWaylandSurfaceColorManager
{
public:
    KisWaylandSurfaceColorManagerWrapper(QObject *parent, const QVariantList &args)
        : KisWaylandSurfaceColorManager(args.first().value<QWindow*>(), parent)
    {
        KIS_SAFE_ASSERT_RECOVER_NOOP(args.size() == 1);
    }
};

} // namespace detail

K_PLUGIN_FACTORY_WITH_JSON(KritaPlatformPluginWaylandFactory, "kritaplatformwayland.json",
    (
        registerPlugin<KisExtendedModifiersMapperWayland>(),
        registerPlugin<detail::KisWaylandSurfaceColorManagerWrapper>(),
        registerPlugin<KisWaylandOutputColorInfo>()
    );)

#include <KritaPlatformPluginWayland.moc>
