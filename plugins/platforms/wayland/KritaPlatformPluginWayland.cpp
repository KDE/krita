/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kpluginfactory.h>

#include <kis_assert.h>

#include <KisExtendedModifiersMapperWayland.h>

#include <config-use-surface-color-management-api.h>

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API

#include <waylandcolormanagement/KisWaylandSurfaceColorManager.h>
#include <waylandcolormanagement/KisWaylandOutputColorInfo.h>
#include <surfacecolormanagement/KisSurfaceColorManagementInfo.h>

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

class KisWaylandSurfaceColorManagementInfo : public KisSurfaceColorManagementInfo
{
public:
    using KisSurfaceColorManagementInfo::KisSurfaceColorManagementInfo;
    bool surfaceColorManagedByOS() override {
        return true;
    }
};

#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */

K_PLUGIN_FACTORY_WITH_JSON(KritaPlatformPluginWaylandFactory, "kritaplatformwayland.json",
    (
        registerPlugin<KisExtendedModifiersMapperWayland>()
#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
        , registerPlugin<detail::KisWaylandSurfaceColorManagerWrapper>()
        , registerPlugin<KisWaylandOutputColorInfo>()
        , registerPlugin<KisWaylandSurfaceColorManagementInfo>()
#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */
    );)

#include <KritaPlatformPluginWayland.moc>
