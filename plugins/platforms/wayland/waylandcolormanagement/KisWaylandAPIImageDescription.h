/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISWAYLANDAPIIMAGEDESCRIPTION_H
#define KISWAYLANDAPIIMAGEDESCRIPTION_H

#include <QWaylandClientExtension>
#include <qwayland-color-management-v1.h>

#include <QString>
#include <QPointF>

#include "KisWaylandSurfaceColorimetry.h"

class KisWaylandAPIImageDescriptionInfo : public QObject, public QtWayland::wp_image_description_info_v1
{
    Q_OBJECT
public:
    explicit KisWaylandAPIImageDescriptionInfo(::wp_image_description_info_v1 *info);

    ~KisWaylandAPIImageDescriptionInfo();

    bool isReady() const {
        return m_isReady;
    }

    Q_SIGNAL void done();

    void wp_image_description_info_v1_done() override;
    void
    wp_image_description_info_v1_primaries(int32_t r_x, int32_t r_y, int32_t g_x, int32_t g_y, int32_t b_x, int32_t b_y, int32_t w_x, int32_t w_y) override;
    void wp_image_description_info_v1_primaries_named(uint32_t primaries) override;
    void wp_image_description_info_v1_tf_power(uint32_t eexp) override;
    void wp_image_description_info_v1_tf_named(uint32_t tf) override;
    void wp_image_description_info_v1_luminances(uint32_t min_lum, uint32_t max_lum, uint32_t reference_lum) override;
    void wp_image_description_info_v1_target_primaries(int32_t r_x, int32_t r_y, int32_t g_x, int32_t g_y, int32_t b_x, int32_t b_y, int32_t w_x, int32_t w_y)
        override;
    void wp_image_description_info_v1_target_luminance(uint32_t min_lum, uint32_t max_lum) override;
    void wp_image_description_info_v1_target_max_cll(uint32_t max_cll) override;
    void wp_image_description_info_v1_target_max_fall(uint32_t max_fall) override;
    void wp_image_description_info_v1_icc_file(int32_t icc, uint32_t icc_size) override;

    KisSurfaceColorimetry::WaylandSurfaceDescription m_data;
    bool m_isReady {false};
};

class KisWaylandAPIImageDescription : public QObject, public QtWayland::wp_image_description_v1
{
    Q_OBJECT
public:
    explicit KisWaylandAPIImageDescription(::wp_image_description_v1 *descr);

    ~KisWaylandAPIImageDescription();

    Q_SIGNAL void done();
    Q_SIGNAL void sigDescriptionConstructed(bool success);

    KisWaylandAPIImageDescriptionInfo info;

    uint32_t identity() const {
        return m_identity;
    }

protected:
    void wp_image_description_v1_failed(uint32_t cause, const QString &msg) override;
    void wp_image_description_v1_ready(uint32_t identity) override;

private:
    uint32_t m_identity {0};
};


#endif /* KISWAYLANDAPIIMAGEDESCRIPTION_H */