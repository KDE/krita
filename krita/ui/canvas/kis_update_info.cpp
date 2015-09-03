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

KisOpenGLUpdateInfo::KisOpenGLUpdateInfo(ConversionOptions options)
    : m_options(options)
{
}

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

bool KisOpenGLUpdateInfo::needsConversion() const
{
    return m_options.m_needsConversion;
}
void KisOpenGLUpdateInfo::convertColorSpace()
{
    KIS_ASSERT_RECOVER_RETURN(needsConversion());

    foreach (KisTextureTileUpdateInfoSP tileInfo, tileList) {
        tileInfo->convertTo(m_options.m_destinationColorSpace,
                            m_options.m_renderingIntent,
                            m_options.m_conversionFlags);
    }
}

#endif /* HAVE_OPENGL */
