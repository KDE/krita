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

QRect KisPPUpdateInfo::dirtyImageRect() const {
    return dirtyImageRectVar;
}

#ifdef HAVE_OPENGL

QRect KisOpenGLUpdateInfo::dirtyViewportRect() {
    qFatal("Not implemented yet!");
    return QRect();
}

void KisOpenGLUpdateInfo::assignDirtyImageRect(const QRect &rect)
{
    m_dirtyImageRect = rect;
}

QRect KisOpenGLUpdateInfo::dirtyImageRect() const
{
    return m_dirtyImageRect;
}

#endif /* HAVE_OPENGL */
