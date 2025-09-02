/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWaylandSurfaceColorManager.h"

#include <QWindow>
#include <QPromise>
#include <QFuture>

#include <qpa/qplatformnativeinterface.h>
#include <qpa/qplatformwindow_p.h>

#include "KisWaylandAPISurface.h"
#include "KisWaylandAPIColorManager.h"
#include "KisWaylandAPIImageDescription.h"
#include "KisWaylandAPIImageDescriptionCreatorParams.h"

#include <kis_debug.h>

namespace {
template <typename T>
QFuture<std::decay_t<T>> makeReadyQFuture(T &&value) {
    QPromise<std::decay_t<T>> promise;
    promise.start();
    promise.addResult(std::forward<T>(value));
    promise.finish();
    return promise.future();
}
}

Q_GLOBAL_STATIC(std::weak_ptr<KisWaylandAPIColorManager>, s_waylandManager)

std::shared_ptr<KisWaylandAPIColorManager> KisWaylandSurfaceColorManager::getOrCreateGlobalWaylandManager()
{
    std::shared_ptr<KisWaylandAPIColorManager> result;

    if (s_waylandManager.exists()) {
        result = s_waylandManager->lock();
    } else {
        result.reset(new KisWaylandAPIColorManager());
        *s_waylandManager = result;
    }

    return result;
}

KisWaylandSurfaceColorManager::KisWaylandSurfaceColorManager(QWindow *window, QObject *parent)
    : KisSurfaceColorManagerInterface(window, parent)
{
    m_waylandManager = getOrCreateGlobalWaylandManager();
    connect(m_waylandManager.get(), &KisWaylandAPIColorManager::sigReadyChanged,
            this, &KisWaylandSurfaceColorManager::reinitialize);

    /**
     * If we have reused an existing wayland manager, then it will be ready
     */
    if (m_waylandManager->isReady()) {
        reinitialize();
    }
}

KisWaylandSurfaceColorManager::~KisWaylandSurfaceColorManager()
{
}

void KisWaylandSurfaceColorManager::setReadyImpl(bool value)
{
    if (value == m_isReady) return;

    m_isReady = value;
    Q_EMIT sigReadyChanged(m_isReady);
}

#include <QPlatformSurfaceEvent>

class WindowIdEventFilter : public QObject
{
    Q_OBJECT
public:
    WindowIdEventFilter(QObject *watched, QObject *parent = nullptr)
        : QObject(parent)
        , m_watched(watched)
    {
        // TODO: autoinstall the filter
    }

Q_SIGNALS:
    void sigWindowIdChanged();

private:
    bool eventFilter(QObject *watched, QEvent *event) override {
        if (watched == m_watched) {
            qDebug() << watched->objectName() << event->type();
        }

        if (watched == m_watched && event->type() == QEvent::PlatformSurface) {
            QPlatformSurfaceEvent *pevent = static_cast<QPlatformSurfaceEvent*>(event);
            if (pevent->surfaceEventType() == QPlatformSurfaceEvent::SurfaceCreated) {
                ENTER_FUNCTION() << "found the event";
                Q_EMIT sigWindowIdChanged();

                // we allow only one-shot
                m_watched = nullptr;

                // dispose of ourselves
                this->deleteLater();
            }
        }

        return false;
    }

    QObject *m_watched = nullptr;
};


void KisWaylandSurfaceColorManager::reinitialize()
{
    ENTER_FUNCTION() << ppVar(m_waylandManager->isReady()) << ppVar(m_window);

    if (!m_waylandManager->isReady()) {
        m_surface.reset();
        m_preferredDescription = std::nullopt;
        m_currentDescription = std::nullopt;
        m_renderingIntent = std::nullopt;
        setReadyImpl(false);
        return;
    }

    auto waylandWindow = m_window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();

    ENTER_FUNCTION() << ppVar(m_window) << ppVar(m_window->handle()) << ppVar(waylandWindow);

    if (!waylandWindow) {
        // the window has not completed its creation yet, just wait a little bit
        auto *filter = new WindowIdEventFilter(m_window, this);
        connect(filter, &WindowIdEventFilter::sigWindowIdChanged, this, &KisWaylandSurfaceColorManager::reinitialize);
        m_window->installEventFilter(filter);
        return;
    }

    auto initializeWaylandSurface = [this, waylandWindow]() {
        disconnect(m_windowConnection);
        if (m_surface) {
            qWarning() << "WARNING: KisWaylandSurfaceColorManager::reinitialize(): surfaceCreated signal got delivered twice!";
            return;
        }

        auto feedback = std::make_unique<KisWaylandAPISurfaceFeedback>(m_waylandManager->get_surface_feedback(waylandWindow->surface()));
        connect(feedback.get(), &KisWaylandAPISurfaceFeedback::preferredChanged, this, &KisWaylandSurfaceColorManager::slotPreferredChanged);
        m_surface = std::make_unique<KisWaylandAPISurface>(m_waylandManager->get_surface(waylandWindow->surface()), std::move(feedback));
    };

    if (waylandWindow->surface()) {
        initializeWaylandSurface();
    } else {
        /**
         * In case the Wayland server got disconnected, wait until the Qt
         * surface is actually created.
         */
        m_windowConnection =
            connect(waylandWindow, &QNativeInterface::Private::QWaylandWindow::surfaceCreated, this, initializeWaylandSurface);
    }
}

bool KisWaylandSurfaceColorManager::isReady() const
{
    return m_isReady;
}

bool KisWaylandSurfaceColorManager::supportsSurfaceDescription(const KisSurfaceColorimetry::SurfaceDescription &desc)
{
    using feature = QtWayland::wp_color_manager_v1::feature;
    using namespace KisColorimetryUtils;

    if (!m_waylandManager->isFeatureSupported(feature::feature_parametric)) {
        qWarning() << "KisWaylandSurfaceColorManager: feature_parametric is not supported";
        return false;
    }

    if (!m_waylandManager->isFeatureSupported(feature::feature_set_primaries) &&
        std::holds_alternative<Colorimetry>(desc.colorSpace.primaries)) {

        qWarning() << "KisWaylandSurfaceColorManager: feature_set_primaries is not supported, even though requested";
        return false;
    }

    if (!m_waylandManager->isFeatureSupported(feature::feature_set_tf_power) &&
        std::holds_alternative<uint32_t>(desc.colorSpace.transferFunction)) {

        qWarning() << "KisWaylandSurfaceColorManager: feature_set_tf_power is not supported, even though requested";
        return false;
    }

    if (!m_waylandManager->isFeatureSupported(feature::feature_set_luminances) &&
        desc.colorSpace.luminance) {

        qWarning() << "KisWaylandSurfaceColorManager: feature_set_luminances is not supported, even though requested";
        return false;
    }

    if (!m_waylandManager->isFeatureSupported(feature::feature_set_mastering_display_primaries) &&
        desc.masteringInfo) {

        qWarning() << "KisWaylandSurfaceColorManager: feature_set_mastering_display_primaries is not supported, even though requested";
        return false;
    }

    if (!m_waylandManager->isFeatureSupported(feature::feature_extended_target_volume) &&
        desc.masteringInfo && desc.colorSpace.luminance) {
        if (desc.masteringInfo->luminance.minLuminance < desc.colorSpace.luminance->minLuminance
            || desc.masteringInfo->luminance.maxLuminance > desc.colorSpace.luminance->maxLuminance) {

            qWarning() << "KisWaylandSurfaceColorManager: feature_set_mastering_display_primaries is not supported, even though requested";
            return false;
        }
    }

    if (std::holds_alternative<KisSurfaceColorimetry::NamedPrimaries>(desc.colorSpace.primaries)) {
        auto waylandPrimaries = primariesKritaToWayland(std::get<KisSurfaceColorimetry::NamedPrimaries>(desc.colorSpace.primaries));

        if (!m_waylandManager->isPrimariesNamedSupported(waylandPrimaries))
            return false;
    }

    if (std::holds_alternative<KisSurfaceColorimetry::NamedTransferFunction>(desc.colorSpace.transferFunction)) {
        auto waylandTransferFunction = transferFunctionKritaToWayland(std::get<KisSurfaceColorimetry::NamedTransferFunction>(desc.colorSpace.transferFunction));

        /**
         * For some obscure reason Wayland compositors implemented transfer_function_srgb
         * as gamma-2.2 transfer function, which caused a lot of confustion. Hence the enum
         * is going to be deprecated in the upcoming version of the protocol.
         *
         * https://gitlab.freedesktop.org/wayland/wayland-protocols/-/merge_requests/442
         */
        if (waylandTransferFunction == QtWayland::wp_color_manager_v1::transfer_function_srgb ||
            waylandTransferFunction == QtWayland::wp_color_manager_v1::transfer_function_ext_srgb) {

            return false;
        }

        if (!m_waylandManager->isTransferFunctionNamedSupported(waylandTransferFunction))
            return false;
    }

    return true;
}

bool KisWaylandSurfaceColorManager::supportsRenderIntent(const KisSurfaceColorimetry::RenderIntent &intent)
{
    auto waylandIntent = renderIntentKritaToWayland(intent);
    return m_waylandManager->isIntentSupported(waylandIntent);
}

#warning USE_KWIN_BUG_WORKAROUND should become optional switch depending on the version of KWin used
#define USE_KWIN_BUG_WORKAROUND

#ifdef USE_KWIN_BUG_WORKAROUND
#include "wayland-wayland-client-protocol.h"
#endif

QFuture<bool> KisWaylandSurfaceColorManager::setSurfaceDescription(const KisSurfaceColorimetry::SurfaceDescription &desc, KisSurfaceColorimetry::RenderIntent intent)
{
    if (!m_isReady) {
        qWarning() << "ERROR: KisWaylandSurfaceColorManager::setSurfaceDescription: the manager is not ready";
        return makeReadyQFuture(false);
    }

    using KisSurfaceColorimetry::WaylandSurfaceDescription;

    auto waylandDescription = WaylandSurfaceDescription::fromSurfaceDescription(desc);
    auto waylandIntent = renderIntentKritaToWayland(intent);

    if (!supportsSurfaceDescription(desc)) {
        qWarning() << "ERROR: KisWaylandSurfaceColorManager::setSurfaceDescription: unsupported surface description";
        return makeReadyQFuture(false);
    }

    if (!m_waylandManager->isIntentSupported(waylandIntent)) {
        qWarning() << "ERROR: KisWaylandSurfaceColorManager::setSurfaceDescription: unsupported rendering intent";
        return makeReadyQFuture(false);
    }

    KisWaylandAPIImageDescriptionCreatorParams creator(m_waylandManager.get());
    std::shared_ptr<KisWaylandAPIImageDescription> descriptionObject =
        creator.createImageDescription(waylandDescription);

    QPromise<bool> imageDescriptionPromise;
    auto future = imageDescriptionPromise.future();

    connect(descriptionObject.get(), &KisWaylandAPIImageDescription::sigDescriptionConstructed,
            this,
            [promise = std::move(imageDescriptionPromise)] (bool success) mutable {
                promise.start();
                promise.addResult(success);
                promise.finish();
            });

    QFuture<bool> result =
        future.then([this, desc, descriptionObject, waylandIntent, intent] (QFuture<bool> future) {
            if (!future.result()) return false;

#ifdef USE_KWIN_BUG_WORKAROUND
            // WARNING: KWin <= 6.4.4 doesn't handle intent changes properly
            if (m_currentDescription && m_currentDescription == desc &&
                m_renderingIntent && m_renderingIntent != intent) {

                m_surface->unset_image_description();
                auto waylandWindow = m_window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
                ::wl_surface_commit(waylandWindow->surface());
            }
#endif

            m_surface->set_image_description(descriptionObject->object(), waylandIntent);
            m_window->requestUpdate();
            m_currentDescription = desc;
            m_renderingIntent = intent;
            return true;
        });

    return result;
}

void KisWaylandSurfaceColorManager::unsetSurfaceDescription()
{
    if (!m_isReady) return;

    m_surface->unset_image_description();
    m_currentDescription = std::nullopt;
    m_renderingIntent = std::nullopt;
}

std::optional<KisSurfaceColorimetry::SurfaceDescription> KisWaylandSurfaceColorManager::surfaceDescription() const
{
    return m_currentDescription;
}

std::optional<KisSurfaceColorimetry::RenderIntent> KisWaylandSurfaceColorManager::renderingIntent() const
{
    return m_renderingIntent;
}

std::optional<KisSurfaceColorimetry::SurfaceDescription> KisWaylandSurfaceColorManager::preferredSurfaceDescription() const
{
    return m_preferredDescription;
}

void KisWaylandSurfaceColorManager::slotPreferredChanged()
{
    auto waylandDesc = m_surface->m_feedback->m_preferred->info.m_data;
    m_preferredDescription = waylandDesc.toSurfaceDescription();

    ENTER_FUNCTION() << ppVar(m_window) << ppVar(m_preferredDescription);

    if (!m_isReady) {
        setReadyImpl(true);
    } else {
        Q_EMIT sigPreferredSurfaceDescriptionChanged(*m_preferredDescription);
    }
}

#include <moc_KisWaylandSurfaceColorManager.cpp>

// TODO: remove
#include <KisWaylandSurfaceColorManager.moc>