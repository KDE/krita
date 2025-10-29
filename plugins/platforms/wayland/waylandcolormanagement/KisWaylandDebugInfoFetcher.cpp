/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisWaylandDebugInfoFetcher.h"

#include <QScreen>
#include <QTimer>
#include <QPointer>

#include <qpa/qplatformnativeinterface.h>

#include "KisWaylandSurfaceColorManager.h"
#include "KisWaylandAPIColorManager.h"

 #include <QtAssert>
 #include <kis_debug.h>

KisWaylandDebugInfoFetcher::KisWaylandDebugInfoFetcher(QObject *parent)
    : QObject(parent)
{
    m_waylandManager = KisWaylandSurfaceColorManager::getOrCreateGlobalWaylandManager();
    connect(m_waylandManager.get(), &KisWaylandAPIColorManager::sigReadyChanged,
            this, &KisWaylandDebugInfoFetcher::reinitialize);
    reinitialize();
}

KisWaylandDebugInfoFetcher::~KisWaylandDebugInfoFetcher()
{
}

bool KisWaylandDebugInfoFetcher::isReady() const
{
    return m_isReady;
}

QString KisWaylandDebugInfoFetcher::report() const
{
    return m_report;
}

void KisWaylandDebugInfoFetcher::reinitialize()
{
    if (m_waylandManager->isReady()) {
        m_report = generateReport();
        m_isReady = true;
        Q_EMIT sigDebugInfoReady(m_report);
    } else {
        m_isReady = false;
        m_report = "";
    }
}

QString KisWaylandDebugInfoFetcher::generateReport()
{
    QString report;

    QDebug s(&report);

    s << "Wayland color management plugin report" << Qt::endl;
    s << Qt::endl;

    if (!m_waylandManager) {
        s << "Wayland manager is not initialized." << Qt::endl;
        return report;
    }

    s << "Supported Features:" << Qt::endl;
    const std::map<QtWayland::wp_color_manager_v1::feature, QString> featureNames = {
        {QtWayland::wp_color_manager_v1::feature_icc_v2_v4, "feature_icc_v2_v4"},
        {QtWayland::wp_color_manager_v1::feature_parametric, "feature_parametric"},
        {QtWayland::wp_color_manager_v1::feature_set_primaries, "feature_set_primaries"},
        {QtWayland::wp_color_manager_v1::feature_set_tf_power, "feature_set_tf_power"},
        {QtWayland::wp_color_manager_v1::feature_set_luminances, "feature_set_luminances"},
        {QtWayland::wp_color_manager_v1::feature_set_mastering_display_primaries, "feature_set_mastering_display_primaries"},
        {QtWayland::wp_color_manager_v1::feature_extended_target_volume, "feature_extended_target_volume"},
        {QtWayland::wp_color_manager_v1::feature_windows_scrgb, "feature_windows_scrgb"}
    };
    for (const auto &[feature, name] : featureNames) {
        s.noquote().nospace() << "    " << name << ": " << (m_waylandManager->isFeatureSupported(feature) ? "yes" : "no") << Qt::endl;
    }

    s << Qt::endl;
    s << "Supported Render Intents:" << Qt::endl;
    const std::map<QtWayland::wp_color_manager_v1::render_intent, QString> intentNames = {
        {QtWayland::wp_color_manager_v1::render_intent_perceptual, "render_intent_perceptual"},
        {QtWayland::wp_color_manager_v1::render_intent_relative, "render_intent_relative"},
        {QtWayland::wp_color_manager_v1::render_intent_saturation, "render_intent_saturation"},
        {QtWayland::wp_color_manager_v1::render_intent_absolute, "render_intent_absolute"},
        {QtWayland::wp_color_manager_v1::render_intent_relative_bpc, "render_intent_relative_bpc"}
    };
    for (const auto &[intent, name] : intentNames) {
        s.noquote().nospace() << "    " << name << ": " << (m_waylandManager->isIntentSupported(intent) ? "yes" : "no") << Qt::endl;
    }

    s << Qt::endl;
    s << "Supported Transfer Functions:" << Qt::endl;
    const std::map<QtWayland::wp_color_manager_v1::transfer_function, QString> tfNames = {
        {QtWayland::wp_color_manager_v1::transfer_function_bt1886, "transfer_function_bt1886"},
        {QtWayland::wp_color_manager_v1::transfer_function_gamma22, "transfer_function_gamma22"},
        {QtWayland::wp_color_manager_v1::transfer_function_gamma28, "transfer_function_gamma28"},
        {QtWayland::wp_color_manager_v1::transfer_function_st240, "transfer_function_st240"},
        {QtWayland::wp_color_manager_v1::transfer_function_ext_linear, "transfer_function_ext_linear"},
        {QtWayland::wp_color_manager_v1::transfer_function_log_100, "transfer_function_log_100"},
        {QtWayland::wp_color_manager_v1::transfer_function_log_316, "transfer_function_log_316"},
        {QtWayland::wp_color_manager_v1::transfer_function_xvycc, "transfer_function_xvycc"},
        {QtWayland::wp_color_manager_v1::transfer_function_srgb, "transfer_function_srgb"},
        {QtWayland::wp_color_manager_v1::transfer_function_ext_srgb, "transfer_function_ext_srgb"},
        {QtWayland::wp_color_manager_v1::transfer_function_st2084_pq, "transfer_function_st2084_pq"},
        {QtWayland::wp_color_manager_v1::transfer_function_st428, "transfer_function_st428"},
        {QtWayland::wp_color_manager_v1::transfer_function_hlg, "transfer_function_hlg"}
    };
    for (const auto &[tf, name] : tfNames) {
        s.noquote().nospace() << "    " << name << ": " << (m_waylandManager->isTransferFunctionNamedSupported(tf) ? "yes" : "no") << Qt::endl;
    }

    s << Qt::endl;
    s << "Supported Primaries:" << Qt::endl;
    const std::map<QtWayland::wp_color_manager_v1::primaries, QString> primaryNames = {
        {QtWayland::wp_color_manager_v1::primaries_srgb, "primaries_srgb"},
        {QtWayland::wp_color_manager_v1::primaries_pal_m, "primaries_pal_m"},
        {QtWayland::wp_color_manager_v1::primaries_pal, "primaries_pal"},
        {QtWayland::wp_color_manager_v1::primaries_ntsc, "primaries_ntsc"},
        {QtWayland::wp_color_manager_v1::primaries_generic_film, "primaries_generic_film"},
        {QtWayland::wp_color_manager_v1::primaries_bt2020, "primaries_bt2020"},
        {QtWayland::wp_color_manager_v1::primaries_cie1931_xyz, "primaries_cie1931_xyz"},
        {QtWayland::wp_color_manager_v1::primaries_dci_p3, "primaries_dci_p3"},
        {QtWayland::wp_color_manager_v1::primaries_display_p3, "primaries_display_p3"},
        {QtWayland::wp_color_manager_v1::primaries_adobe_rgb, "primaries_adobe_rgb"}
    };
    for (const auto &[primary, name] : primaryNames) {
        s.noquote().nospace() << "    " << name << ": " << (m_waylandManager->isPrimariesNamedSupported(primary) ? "yes" : "no") << Qt::endl;
    }

    return report;
}

#include <moc_KisWaylandDebugInfoFetcher.cpp>