/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWaylandAPIImageDescription.h"

using namespace Qt::Literals::StringLiterals;


KisWaylandAPIImageDescriptionInfo::KisWaylandAPIImageDescriptionInfo(::wp_image_description_info_v1 *info)
    : QtWayland::wp_image_description_info_v1(info)
{
}
KisWaylandAPIImageDescriptionInfo::~KisWaylandAPIImageDescriptionInfo()
{
    wp_image_description_info_v1_destroy(object());
}
void KisWaylandAPIImageDescriptionInfo::wp_image_description_info_v1_done()
{
    m_isReady = true;
    Q_EMIT done();
}

void KisWaylandAPIImageDescriptionInfo::wp_image_description_info_v1_primaries(int32_t r_x,
                                                                                      int32_t r_y,
                                                                                      int32_t g_x,
                                                                                      int32_t g_y,
                                                                                      int32_t b_x,
                                                                                      int32_t b_y,
                                                                                      int32_t w_x,
                                                                                      int32_t w_y)
{
    using KisSurfaceColorimetry::WaylandPrimaries;
    using KisSurfaceColorimetry::xyFromWaylandXy;

    WaylandPrimaries colorPrimaries;
    colorPrimaries.red = xyFromWaylandXy(r_x, r_y);
    colorPrimaries.green = xyFromWaylandXy(g_x, g_y);
    colorPrimaries.blue = xyFromWaylandXy(b_x, b_y);
    colorPrimaries.white = xyFromWaylandXy(w_x, w_y);
    m_data.container = colorPrimaries;
}

void KisWaylandAPIImageDescriptionInfo::wp_image_description_info_v1_primaries_named(uint32_t primaries)
{
    using KisSurfaceColorimetry::WaylandSurfaceDescription;

    Q_ASSERT(!m_data.namedContainer);
    m_data.namedContainer = WaylandSurfaceDescription::primaries(primaries);
}

void KisWaylandAPIImageDescriptionInfo::wp_image_description_info_v1_tf_power(uint32_t eexp)
{
    Q_ASSERT(!m_data.tfGamma);
    m_data.tfGamma = eexp;
}

void KisWaylandAPIImageDescriptionInfo::wp_image_description_info_v1_tf_named(uint32_t tf)
{
    using KisSurfaceColorimetry::WaylandSurfaceDescription;

    Q_ASSERT(!m_data.tfNamed);
    m_data.tfNamed = WaylandSurfaceDescription::transfer_function(tf);
}

void KisWaylandAPIImageDescriptionInfo::wp_image_description_info_v1_luminances(uint32_t min_lum, uint32_t max_lum, uint32_t reference_lum)
{
    m_data.luminances = {min_lum, max_lum, reference_lum};
}

void KisWaylandAPIImageDescriptionInfo::wp_image_description_info_v1_target_primaries(int32_t r_x,
                                                                                             int32_t r_y,
                                                                                             int32_t g_x,
                                                                                             int32_t g_y,
                                                                                             int32_t b_x,
                                                                                             int32_t b_y,
                                                                                             int32_t w_x,
                                                                                             int32_t w_y)

{
    using KisSurfaceColorimetry::WaylandPrimaries;
    using KisSurfaceColorimetry::xyFromWaylandXy;

    WaylandPrimaries colorPrimaries;
    colorPrimaries.red = xyFromWaylandXy(r_x, r_y);
    colorPrimaries.green = xyFromWaylandXy(g_x, g_y);
    colorPrimaries.blue = xyFromWaylandXy(b_x, b_y);
    colorPrimaries.white = xyFromWaylandXy(w_x, w_y);
    m_data.target = colorPrimaries;
}

void KisWaylandAPIImageDescriptionInfo::wp_image_description_info_v1_target_luminance(uint32_t min_lum, uint32_t max_lum)
{
    m_data.masteringLuminance = {min_lum, max_lum};
}

void KisWaylandAPIImageDescriptionInfo::wp_image_description_info_v1_target_max_cll(uint32_t max_cll)
{
    m_data.targetMaxCLL = max_cll;
}

void KisWaylandAPIImageDescriptionInfo::wp_image_description_info_v1_target_max_fall(uint32_t max_fall)
{
    m_data.targetMaxFALL = max_fall;
}

void KisWaylandAPIImageDescriptionInfo::wp_image_description_info_v1_icc_file(int32_t icc, uint32_t icc_size)
{
    Q_UNUSED(icc)
    Q_UNUSED(icc_size)
    m_data.iccFileIsPresent = true;
    qWarning() << "WARNING: wp_image_description_info_v1_icc_file was received, but we don't support ICC files for the surface description!";
}

KisWaylandAPIImageDescription::KisWaylandAPIImageDescription(::wp_image_description_v1 *descr)
    : QtWayland::wp_image_description_v1(descr)
    , info(get_information())
{
    connect(&info, &KisWaylandAPIImageDescriptionInfo::done, this, &KisWaylandAPIImageDescription::done);
}

KisWaylandAPIImageDescription::~KisWaylandAPIImageDescription()
{
    wp_image_description_v1_destroy(object());
}

void KisWaylandAPIImageDescription::wp_image_description_v1_failed(uint32_t cause, const QString &msg)
{
    using cause_type = QtWayland::wp_image_description_v1::cause;

    cause_type realCause = static_cast<cause_type>(cause);
    QLatin1String causeText;

    switch (realCause) {
        case cause_low_version:
            causeText = "cause_low_version"_L1;
            break;
        case cause_unsupported:
            causeText = "cause_unsupported"_L1;
            break;
        case cause_operating_system:
            causeText = "cause_operating_system"_L1;
            break;
        case cause_no_output:
            causeText = "cause_no_output"_L1;
            break;
    }

    qWarning() << "KisWaylandImageDescription: Failed to create image description:" << causeText << ": " << msg;
    Q_EMIT sigDescriptionConstructed(false);
}

void KisWaylandAPIImageDescription::wp_image_description_v1_ready(uint32_t identity)
{
    m_identity = identity;
    Q_EMIT sigDescriptionConstructed(true);
}

#include "moc_KisWaylandAPIImageDescription.cpp"