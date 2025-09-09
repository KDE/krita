/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISWAYLANDSURFACECOLORMANAGER_H
#define KISWAYLANDSURFACECOLORMANAGER_H

#include <QPointer>

#include <surfacecolormanagement/KisSurfaceColorManagerInterface.h>

class KisWaylandAPIColorManager;
class KisWaylandAPISurface;

class KisWaylandSurfaceColorManager : public KisSurfaceColorManagerInterface
{
    Q_OBJECT
public:
    KisWaylandSurfaceColorManager(QWindow *window, QObject *parent = nullptr);
    ~KisWaylandSurfaceColorManager() override;

    bool isReady() const override;
    bool supportsSurfaceDescription(const KisSurfaceColorimetry::SurfaceDescription &desc) override;
    bool supportsRenderIntent(const KisSurfaceColorimetry::RenderIntent &intent) override;
    QFuture<bool> setSurfaceDescription(const KisSurfaceColorimetry::SurfaceDescription &desc, KisSurfaceColorimetry::RenderIntent intent) override;
    void unsetSurfaceDescription() override;
    std::optional<KisSurfaceColorimetry::SurfaceDescription> surfaceDescription() const override;
    std::optional<KisSurfaceColorimetry::RenderIntent> renderingIntent() const override;
    std::optional<KisSurfaceColorimetry::SurfaceDescription> preferredSurfaceDescription() const override;

    static std::shared_ptr<KisWaylandAPIColorManager> getOrCreateGlobalWaylandManager();

    enum class WaylandSurfaceState {
        Disconnected = 0, // 1) the underlying wayland manager is inactive
        Connected, // 1) the wayland manager is active; 2) m_platformWindowStateDetector is connected
        WaylandWindowCreated, // 1) QWaylandWindow is created; 2) surfaceCreated() and surfaceDestroyed() signals are connected
        WaylandSurfaceCreated, // 1) wayland surface is created inside Qt
        APIFeedbackCreated, // 1) surface feedback (m_surface) is created and connected to the wayland surface
        PreferredDescriptionReceived // 1) m_preferredDescription is initialized
    };
    Q_ENUM(WaylandSurfaceState)

private Q_SLOTS:
    void slotPreferredChanged();

    void slotPlatformWindowCreated();
    void slotPlatformWindowDestroyed();

    void slotWaylandSurfaceCreated();
    void slotWaylandSurfaceDestroyed();

private:
    void reinitialize();
    void setReadyImpl(bool value);

    WaylandSurfaceState tryInitilize();
    WaylandSurfaceState tryDeinitialize(std::optional<KisWaylandSurfaceColorManager::WaylandSurfaceState> targetState);

private:
    WaylandSurfaceState m_currentState {WaylandSurfaceState::Disconnected};
    std::shared_ptr<KisWaylandAPIColorManager> m_waylandManager;
    std::unique_ptr<KisWaylandAPISurface> m_surface;

    std::optional<KisSurfaceColorimetry::SurfaceDescription> m_currentDescription;
    std::optional<KisSurfaceColorimetry::RenderIntent> m_renderingIntent;
    std::optional<KisSurfaceColorimetry::SurfaceDescription> m_preferredDescription;

    bool m_isReady {false};

    QMetaObject::Connection m_surfaceCreatedConnection;
    QMetaObject::Connection m_surfaceDestroyedConnection;
    QPointer<QObject> m_platformWindowStateDetector;
};

#endif /* KISWAYLANDSURFACECOLORMANAGER_H */