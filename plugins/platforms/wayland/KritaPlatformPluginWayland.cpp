/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWaylandRegistry.h"
#include "qwayland-color-management-v1.h"
#include <kpluginfactory.h>

#include <kis_assert.h>

#include <KisExtendedModifiersMapperWayland.h>

#include <config-use-surface-color-management-api.h>

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API

#include <waylandcolormanagement/KisWaylandSurfaceColorManager.h>
#include <waylandcolormanagement/KisWaylandOutputColorInfo.h>
#include <surfacecolormanagement/KisSurfaceColorManagementInfo.h>
#include <waylandcolormanagement/KisWaylandDebugInfoFetcher.h>

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
    Q_OBJECT
public:
    using KisSurfaceColorManagementInfo::KisSurfaceColorManagementInfo;
    bool surfaceColorManagedByOS() override {
        KisWaylandRegistry* registry = KisWaylandRegistry::getOrCreate();
        return registry->globalExists(QtWayland::wp_color_manager_v1::interface()->name);
    }

    QFuture<QString> debugReport() override {
        QPromise<QString> promise;
        promise.start();

        std::shared_ptr<KisWaylandDebugInfoFetcher> infoFetcher(new KisWaylandDebugInfoFetcher());

        if (infoFetcher->isReady()) {
            promise.addResult(infoFetcher->report());
            promise.finish();

            return promise.future();
        } else {
            // wrap the info fetcher into the resulting lambda
            QFuture<QString> result = promise.future().then(
                [infoFetcher] (const QString &report) {
                    // a simple wrapping class that holds the fetcher until the
                    // result is reported or the future is destroyed
                    return report;
            });

            // move the promise into the handler of its completion signal
            connect(infoFetcher.get(), &KisWaylandDebugInfoFetcher::sigDebugInfoReady,
                    infoFetcher.get(), [promise = std::move(promise)] (const QString &report) mutable {
                        promise.addResult(report);
                        promise.finish();
                    });

            return result;
        }
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

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
// for detail::KisWaylandSurfaceColorManagerWrapper
#include <KritaPlatformPluginWayland.moc>
#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */
