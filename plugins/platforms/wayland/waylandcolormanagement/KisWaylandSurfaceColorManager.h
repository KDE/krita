/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISWAYLANDSURFACECOLORMANAGER_H
#define KISWAYLANDSURFACECOLORMANAGER_H

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

private Q_SLOTS:
    void slotPreferredChanged();

private:
    void reinitialize();
    void setReadyImpl(bool value);

private:
    std::shared_ptr<KisWaylandAPIColorManager> m_waylandManager;
    std::unique_ptr<KisWaylandAPISurface> m_surface;

    std::optional<KisSurfaceColorimetry::SurfaceDescription> m_currentDescription;
    std::optional<KisSurfaceColorimetry::RenderIntent> m_renderingIntent;
    std::optional<KisSurfaceColorimetry::SurfaceDescription> m_preferredDescription;

    bool m_isReady {false};

    QMetaObject::Connection m_windowConnection;
};

#endif /* KISWAYLANDSURFACECOLORMANAGER_H */