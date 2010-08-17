#include "kis_update_info.h"

/**
 * The connection in KisCanvas2 uses queued signals
 * with an argument of KisNodeSP type, so we should
 * register it beforehand
 */
struct KisUpdateInfoSPStaticRegistrar {
    KisUpdateInfoSPStaticRegistrar() {
        qRegisterMetaType<KisUpdateInfoSP>("KisUpdateInfoSP");
    }
};
static KisUpdateInfoSPStaticRegistrar __registrar;

KisUpdateInfo::KisUpdateInfo()
{
}

KisUpdateInfo::~KisUpdateInfo()
{
}

QRect KisUpdateInfo::dirtyViewportRect()
{
    return QRect();
}

QRect KisPPUpdateInfo::dirtyViewportRect() {
    return viewportRect.toAlignedRect();
}


#ifdef HAVE_OPENGL

QRect KisOpenGLUpdateInfo::dirtyViewportRect() {
    qFatal("Not implemented yet!");
    return QRect();
}

#endif /* HAVE_OPENGL */
