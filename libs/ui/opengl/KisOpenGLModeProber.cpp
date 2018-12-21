#include "KisOpenGLModeProber.h"

#include <QGlobalStatic>
Q_GLOBAL_STATIC(KisOpenGLModeProber, s_instance)


KisOpenGLModeProber::KisOpenGLModeProber()
{
}

KisOpenGLModeProber::~KisOpenGLModeProber()
{

}

KisOpenGLModeProber *KisOpenGLModeProber::instance()
{
    return s_instance;
}

bool KisOpenGLModeProber::useHDRMode() const
{
    return isFormatHDR(QSurfaceFormat::defaultFormat());
}

QSurfaceFormat KisOpenGLModeProber::surfaceformatInUse() const
{
    return QSurfaceFormat::defaultFormat();
}

void KisOpenGLModeProber::initSurfaceFormatFromConfig(KisConfig::RootSurfaceFormat config,
                                                      QSurfaceFormat *format)
{
    if (config == KisConfig::BT2020_PQ) {
        format->setRedBufferSize(10);
        format->setGreenBufferSize(10);
        format->setBlueBufferSize(10);
        format->setAlphaBufferSize(2);
        format->setColorSpace(QSurfaceFormat::bt2020PQColorSpace);
    } else if (config == KisConfig::BT709_G10) {
        format->setRedBufferSize(16);
        format->setGreenBufferSize(16);
        format->setBlueBufferSize(16);
        format->setAlphaBufferSize(16);
        format->setColorSpace(QSurfaceFormat::scRGBColorSpace);
    } else {
        format->setRedBufferSize(8);
        format->setGreenBufferSize(8);
        format->setBlueBufferSize(8);
        format->setAlphaBufferSize(8);
        // TODO: check if we can use real sRGB space here
        format->setColorSpace(QSurfaceFormat::DefaultColorSpace);
    }
}

bool KisOpenGLModeProber::isFormatHDR(const QSurfaceFormat &format)
{
    bool isBt2020PQ =
        format.colorSpace() == QSurfaceFormat::bt2020PQColorSpace &&
        format.redBufferSize() == 10 &&
        format.greenBufferSize() == 10 &&
        format.blueBufferSize() == 10 &&
        format.alphaBufferSize() == 10;

    bool isBt709G10 =
        format.colorSpace() == QSurfaceFormat::scRGBColorSpace &&
        format.redBufferSize() == 16 &&
        format.greenBufferSize() == 16 &&
        format.blueBufferSize() == 16 &&
        format.alphaBufferSize() == 16;


    return isBt2020PQ || isBt709G10;
}
