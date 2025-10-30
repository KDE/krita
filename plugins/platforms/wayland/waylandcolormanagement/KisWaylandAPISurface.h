/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISWAYLANDAPISURFACE_H
#define KISWAYLANDAPISURFACE_H

#include <QObject>
#include <qwayland-color-management-v1.h>

class KisWaylandAPIImageDescription;

class KisWaylandAPISurfaceFeedback : public QObject, public QtWayland::wp_color_management_surface_feedback_v1
{
    Q_OBJECT
public:
    explicit KisWaylandAPISurfaceFeedback(::wp_color_management_surface_feedback_v1 *obj);

    ~KisWaylandAPISurfaceFeedback();

    Q_SIGNAL void preferredChanged();
    void wp_color_management_surface_feedback_v1_preferred_changed(uint32_t identity) override;

    std::unique_ptr<KisWaylandAPIImageDescription> m_preferred;

private:
    void requestPreferredDescriptionInfo();
};

class KisWaylandAPISurface : public QObject, public QtWayland::wp_color_management_surface_v1
{
    Q_OBJECT
public:
    explicit KisWaylandAPISurface(::wp_color_management_surface_v1 *obj, std::unique_ptr<KisWaylandAPISurfaceFeedback> &&feedback);
    ~KisWaylandAPISurface();

    std::unique_ptr<KisWaylandAPISurfaceFeedback> m_feedback;
};

#endif /* KISWAYLANDAPISURFACE_H */