/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisMultiSurfaceStateManager.h"

#include <KisView.h>

#include <opengl/KisOpenGLModeProber.h>
#include <KisPlatformPluginInterfaceFactory.h>

#include <KisRootSurfaceInfoProxy.h>


KisMultiSurfaceStateManager::KisMultiSurfaceStateManager()
{
}

KisMultiSurfaceStateManager::~KisMultiSurfaceStateManager()
{
}

#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
void KisMultiSurfaceStateManager::setRootSurfaceInfoProxy(KisRootSurfaceInfoProxy *proxy)
{
    m_rootSurfaceInfoProxy = proxy;
}
#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */

KisMultiSurfaceStateManager::State KisMultiSurfaceStateManager::createInitializingConfig(bool isCanvasOpenGL, int screenId, KisProofingConfigurationSP proofingConfig) const
{
    State state;
    KisMultiSurfaceDisplayConfig &multiConfig = state.multiConfig;

    state.isCanvasOpenGL = isCanvasOpenGL;

    KisConfig cfg(true);

    state.optionsFromConfig = KisDisplayConfig::optionsFromKisConfig(cfg);
    state.surfaceMode = cfg.canvasSurfaceColorSpaceManagementMode();
    state.proofingConfig = proofingConfig;

    if (KisOpenGLModeProber::instance()->useHDRMode()) {
        multiConfig.canvasProfile = state.isCanvasOpenGL
            ? KisOpenGLModeProber::instance()->rootSurfaceColorProfile()
            : KoColorSpaceRegistry::instance()->p709SRGBProfile();
        multiConfig.uiProfile = KoColorSpaceRegistry::instance()->p709SRGBProfile();

    } else if (KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS()) {
#if KRITA_USE_SURFACE_COLOR_MANAGEMENT_API
        KIS_SAFE_ASSERT_RECOVER_NOOP(m_rootSurfaceInfoProxy);
        if (m_rootSurfaceInfoProxy) {
            multiConfig.canvasProfile = m_rootSurfaceInfoProxy->rootSurfaceProfile();
            multiConfig.uiProfile = m_rootSurfaceInfoProxy->rootSurfaceProfile();
        }
#else
        KIS_SAFE_ASSERT_RECOVER_NOOP(0 && "managed surface mode is active, but Krita is compiled without it!");
        multiConfig.canvasProfile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
        multiConfig.uiProfile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
#endif /* KRITA_USE_SURFACE_COLOR_MANAGEMENT_API */
    } else {
        const KoColorProfile *profile = cfg.displayProfile(screenId);
        KIS_SAFE_ASSERT_RECOVER(profile) {
            profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
        }

        multiConfig.canvasProfile = profile;
        multiConfig.uiProfile = profile;
    }

    multiConfig.setOptions(overriddenWithProofingConfig(state.optionsFromConfig, state.proofingConfig));
    multiConfig.isCanvasHDR = false;

    return state;
}

KisMultiSurfaceStateManager::State KisMultiSurfaceStateManager::onCanvasSurfaceFormatChanged(const State &oldState,
                                                                                           const KisDisplayConfig &canvasConfig) const
{
    /// Surface changed signal is possible only for the managed surface under
    /// an openGL canvas. In legacy HDR mode it is not possible to get that.
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS(), oldState);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!KisOpenGLModeProber::instance()->useHDRMode(), oldState);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(oldState.isCanvasOpenGL, oldState);

    // the surface format change should not change the conversion
    // options of the surface
    // TODO: downgrade to a warning!
    KIS_SAFE_ASSERT_RECOVER_NOOP(oldState.multiConfig.options() == canvasConfig.options());

    State newState = oldState;
    KisMultiSurfaceDisplayConfig &newMultiConfig = newState.multiConfig;

    newMultiConfig.canvasProfile = canvasConfig.profile;
    newMultiConfig.setOptions(canvasConfig.options());
    newMultiConfig.isCanvasHDR = canvasConfig.isHDR;

    return newState;
}

KisMultiSurfaceStateManager::State KisMultiSurfaceStateManager::onGuiSurfaceFormatChanged(const State &oldState,
                                                                                        const KoColorProfile *uiProfile) const
{
    /// Surface changed signal is possible only on OS with managed surface
    /// color space
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS(), oldState);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(!KisOpenGLModeProber::instance()->useHDRMode(), oldState);

    State newState = oldState;
    newState.multiConfig.uiProfile = uiProfile;

    return newState;
}

KisMultiSurfaceStateManager::State KisMultiSurfaceStateManager::onProofingChanged(const State &oldState, KisProofingConfigurationSP proofingConfig) const
{
    State newState = oldState;
    newState.proofingConfig = proofingConfig;
    newState.multiConfig.setOptions(
        overriddenWithProofingConfig(newState.optionsFromConfig, newState.proofingConfig));
    return newState;
}

KisMultiSurfaceStateManager::State KisMultiSurfaceStateManager::onConfigChanged(const State &oldState,
                                                                               int screenId,
                                                                               KisConfig::CanvasSurfaceMode surfaceMode,
                                                                               const KisDisplayConfig::Options &options) const
{
    State newState = oldState;
    newState.surfaceMode = surfaceMode;
    newState.optionsFromConfig = options;
    newState.multiConfig.setOptions(
        overriddenWithProofingConfig(newState.optionsFromConfig, newState.proofingConfig));

    if (!KisOpenGLModeProber::instance()->useHDRMode() &&
        !KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS()) {

        KisConfig cfg(true);
        const KoColorProfile *profile = cfg.displayProfile(screenId);
        KIS_SAFE_ASSERT_RECOVER(profile) {
            profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
        }
        newState.multiConfig.canvasProfile = profile;
        newState.multiConfig.uiProfile = profile;
    }

    return newState;
}

KisMultiSurfaceStateManager::State KisMultiSurfaceStateManager::onScreenChanged(const State &oldState, int screenId) const
{
    if (KisOpenGLModeProber::instance()->useHDRMode() ||
        KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS()) {
        return oldState;
    }

    return onConfigChanged(oldState, screenId, oldState.surfaceMode, oldState.optionsFromConfig);
}

KisDisplayConfig::Options KisMultiSurfaceStateManager::overriddenWithProofingConfig(const KisDisplayConfig::Options &options, KisProofingConfigurationSP proofingConfig) const
{
    if (proofingConfig && proofingConfig->displayFlags.testFlag(KoColorConversionTransformation::SoftProofing)) {
        return { proofingConfig->determineDisplayIntent(options.first),
                 proofingConfig->determineDisplayFlags(options.second) };
    }

    return options;
}
