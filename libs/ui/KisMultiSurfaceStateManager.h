/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISMULTISURFACESTATEMANAGER_H
#define KISMULTISURFACESTATEMANAGER_H

#include <kis_config.h>
#include <KisDisplayConfig.h>
#include <KisProofingConfiguration.h>

#include <config-use-surface-color-management-api.h>

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API

#include <QPointer>
class KisRootSurfaceInfoProxy;

#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */



/**
 * On some systems the canvas may have a surface format different
 * from the rest of the GUI. This class help to manage the state
 * of both the surfaces, GUI and canvas ones in an atomic way using
 * value-based semantics.
 *
 * Every incoming event accept the current state and transitions
 * it forward into a new state that is returned from the function.
 * The calling code should only check for the changes and apply
 * the said changes to the actual consumers: KisDisplayColorConverter,
 * KisOpenGLCanvas2 and KisCanvasSurfaceColorSpaceManager.
 *
 * Handling the state in a value-based atomic way allows us to avoid
 * weird flow of updates between different consumers. Such flows are
 * almost impossible to manage externally because we have three different
 * canvas managemend modes:
 *
 * 1) Legacy HDR mode (the one used on Windows), where the whole main
 *    window's surface is switched into the HDR space and compositing
 *    happens inside Qt
 *
 * 2) Wayland-style HDR mode, where the canvas has its own surface, which
 *    color space is managed by OS
 *
 * 3) Unmanaged mode, where we assume OS just passes color data directly
 *    to the GPU, so we perform all the color management ourselves.
 */
class KisMultiSurfaceStateManager
{
public:
    struct State {
        bool isCanvasOpenGL = false;
        KisProofingConfigurationSP proofingConfig;
        KisConfig::CanvasSurfaceMode surfaceMode;
        KisDisplayConfig::Options optionsFromConfig;
        KisMultiSurfaceDisplayConfig multiConfig;

        bool operator==(const State& other) const {
            // TODO: check actual content of proofingConfig
            return isCanvasOpenGL == other.isCanvasOpenGL &&
                   proofingConfig == other.proofingConfig &&
                   surfaceMode == other.surfaceMode &&
                   optionsFromConfig == other.optionsFromConfig &&
                   multiConfig == other.multiConfig;
        }

        bool operator!=(const State& other) const {
            return !(*this == other);
        }
    };

public:
    KisMultiSurfaceStateManager();
    ~KisMultiSurfaceStateManager();

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    void setRootSurfaceInfoProxy(KisRootSurfaceInfoProxy *proxy);
#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */

    State createInitializingConfig(bool isCanvasOpenGL, int screenId, KisProofingConfigurationSP proofingConfig) const;

    State onCanvasSurfaceFormatChanged(const State &oldState,
                                       const KisDisplayConfig &canvasConfig) const;

    State onGuiSurfaceFormatChanged(const State &oldState,
                                    const KoColorProfile *uiProfile) const;

    State onProofingChanged(const State &oldState, KisProofingConfigurationSP proofingConfig) const;

    State onConfigChanged(const State &oldState,
                          int screenId,
                          KisConfig::CanvasSurfaceMode surfaceMode,
                          const KisDisplayConfig::Options &options) const;

    State onScreenChanged(const State &oldState, int screenId) const;

private:
    KisDisplayConfig::Options overriddenWithProofingConfig(const KisDisplayConfig::Options &options, KisProofingConfigurationSP proofingConfig) const;

private:
#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
    QPointer<KisRootSurfaceInfoProxy> m_rootSurfaceInfoProxy;
#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */
};


#endif /* KISMULTISURFACESTATEMANAGER_H */
