/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisSRGBSurfaceColorSpaceManager.h"

#include <QWidget>
#include <QWindow>

#include <kis_assert.h>
#include <kis_config.h>
#include <kis_config_notifier.h>

#include <KisPlatformPluginInterfaceFactory.h>
#include <surfacecolormanagement/KisSurfaceColorManagerInterface.h>


KisSRGBSurfaceColorSpaceManager::KisSRGBSurfaceColorSpaceManager(KisSurfaceColorManagerInterface *interface, QObject *parent)
    : QObject(parent)
    , m_interface(interface)
{
    connect(m_interface.data(), &KisSurfaceColorManagerInterface::sigReadyChanged, this, &KisSRGBSurfaceColorSpaceManager::slotInterfaceReadyChanged);
    connect(KisConfigNotifier::instance(), &KisConfigNotifier::configChanged, this, &KisSRGBSurfaceColorSpaceManager::slotConfigChanged);
}

KisSRGBSurfaceColorSpaceManager::~KisSRGBSurfaceColorSpaceManager()
{
}

QString KisSRGBSurfaceColorSpaceManager::osPreferredColorSpaceReport() const
{
    QString report;
    QDebug str(&report);

    if (!m_interface->isReady()) {
        str << "WARNING: surface color management interface is not ready!" << Qt::endl;
        str << Qt::endl;
    }

    if (m_interface->preferredSurfaceDescription()) {
        str << Qt::endl;
        str.noquote() << m_interface->preferredSurfaceDescription()->makeTextReport() << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }

    return report;
}

QString KisSRGBSurfaceColorSpaceManager::colorManagementReport() const
{
    QString report;
    QDebug str(&report);

    str << "(sRGB surface color manager)" << Qt::endl;
    str << Qt::endl;

    if (!m_interface->isReady()) {
        str << "WARNING: surface color management interface is not ready!" << Qt::endl;
        str << Qt::endl;
    }

    using KisSurfaceColorimetry::RenderIntent;
    using KisSurfaceColorimetry::SurfaceDescription;
    using KisSurfaceColorimetry::NamedPrimaries;
    using KisSurfaceColorimetry::NamedTransferFunction;

    RenderIntent preferredIntent = calculateConfigIntent();
    str << "Requested intent:" << preferredIntent << "supported:" << m_interface->supportsRenderIntent(preferredIntent) << Qt::endl;
    str << "Actual intent:";
    if (m_interface->renderingIntent()) {
        str << *m_interface->renderingIntent() << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }
    str << Qt::endl;

    str << "Active surface description:";
    if (m_interface->surfaceDescription()) {
        str << Qt::endl;
        str.noquote() << m_interface->surfaceDescription()->makeTextReport() << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }
    str << Qt::endl;

    str << "Compositor preferred surface description:";
    if (m_interface->preferredSurfaceDescription()) {
        str << Qt::endl;
        str.noquote() << m_interface->preferredSurfaceDescription()->makeTextReport() << Qt::endl;
    } else {
        str << "<none>" << Qt::endl;
    }

    return report;
}

KisSurfaceColorimetry::RenderIntent KisSRGBSurfaceColorSpaceManager::calculateConfigIntent() {
    using KisSurfaceColorimetry::RenderIntent;

    KisConfig cfg(true);
    RenderIntent intent = RenderIntent::render_intent_perceptual;

    switch (cfg.monitorRenderIntent()) {
        case INTENT_PERCEPTUAL:
            // default value
            break;
        case INTENT_RELATIVE_COLORIMETRIC:
            intent =
                cfg.useBlackPointCompensation() ?
                RenderIntent::render_intent_relative_bpc :
                RenderIntent::render_intent_relative;
            break;
        case INTENT_SATURATION:
            intent =
                RenderIntent::render_intent_saturation;
            break;
        case INTENT_ABSOLUTE_COLORIMETRIC:
            intent =
                RenderIntent::render_intent_absolute;
            break;
    }
    return intent;
}

void KisSRGBSurfaceColorSpaceManager::slotConfigChanged() {
    if (m_interface->isReady() && calculateConfigIntent() != m_interface->renderingIntent()) {
        reinitializeSurfaceDescription();
    }
}

void KisSRGBSurfaceColorSpaceManager::slotInterfaceReadyChanged(bool isReady) {
    if (isReady) {
        reinitializeSurfaceDescription();
    }
}

void KisSRGBSurfaceColorSpaceManager::reinitializeSurfaceDescription() {
    using KisSurfaceColorimetry::RenderIntent;
    using KisSurfaceColorimetry::SurfaceDescription;
    using KisSurfaceColorimetry::NamedPrimaries;
    using KisSurfaceColorimetry::NamedTransferFunction;

    RenderIntent preferredIntent = calculateConfigIntent();

    if (!m_interface->supportsRenderIntent(preferredIntent)) {
        qWarning() << "WARNING: failed to set user preferred rendering"
                   << "intent for the surface, intent \""
                   << preferredIntent << "\" is unsupported, falling back to \"perceptual\"";

        preferredIntent = RenderIntent::render_intent_perceptual;

        // perceptual intent is guaranteed to be supported by the compositor
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_interface->supportsRenderIntent(preferredIntent));
    }

    SurfaceDescription preferredDescription;
    preferredDescription.colorSpace.primaries = NamedPrimaries::primaries_srgb;
    preferredDescription.colorSpace.transferFunction = NamedTransferFunction::transfer_function_srgb;


    if (!m_interface->supportsSurfaceDescription(preferredDescription)) {
        preferredDescription.colorSpace.transferFunction = NamedTransferFunction::transfer_function_gamma22;
    }

    if (m_interface->supportsSurfaceDescription(preferredDescription)) {
        auto future = m_interface->setSurfaceDescription(preferredDescription, preferredIntent);
        future.then([] (QFuture<bool> result) {
            if (!result.isValid() || !result.result()) {
                qWarning() << "WARNING: failed to set sRGB color space for the surface, setSurfaceDescription() returned false";
            }
        });
    } else {
        qWarning() << "WARNING: failed to set sRGB color space for the surface,"
                   << "the color space is unsupported by compositor";
    }
}

KisSRGBSurfaceColorSpaceManager* KisSRGBSurfaceColorSpaceManager::tryCreateForCurrentPlatform(QWidget *widget)
{
    QWindow *nativeWindow = widget->windowHandle();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(widget->windowHandle(), nullptr);

    std::unique_ptr<KisSurfaceColorManagerInterface> iface(
        KisPlatformPluginInterfaceFactory::instance()->createSurfaceColorManager(widget->windowHandle()));

    if (iface) {
        return new KisSRGBSurfaceColorSpaceManager(iface.release(), nativeWindow);
    }

    return nullptr;
}

#include <moc_KisSRGBSurfaceColorSpaceManager.cpp>
