/*
 *  Copyright (c) 2017 Alvin Wong <alvinhochun@gmail.com>
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "KisOpenGLModeProber.h"

#include <config-hdr.h>
#include <QApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QWindow>

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
    // TODO: use information provided by KisOpenGL instead
    QOpenGLContext *sharedContext = QOpenGLContext::globalShareContext();
    QSurfaceFormat format = sharedContext ? sharedContext->format() : QSurfaceFormat::defaultFormat();
    return format;
}

const KoColorProfile *KisOpenGLModeProber::rootSurfaceColorProfile() const
{
    const QSurfaceFormat::ColorSpace surfaceColorSpace = surfaceformatInUse().colorSpace();
    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();

    if (surfaceColorSpace == QSurfaceFormat::sRGBColorSpace) {
        // use the default one!
#ifdef HAVE_HDR
    } else if (surfaceColorSpace == QSurfaceFormat::scRGBColorSpace) {
        profile = KoColorSpaceRegistry::instance()->p709G10Profile();
    } else if (surfaceColorSpace == QSurfaceFormat::bt2020PQColorSpace) {
        profile = KoColorSpaceRegistry::instance()->p2020PQProfile();
#endif
    }

    return profile;
}

namespace {
struct AppAttributeSetter
{
    AppAttributeSetter(Qt::ApplicationAttribute attribute, bool useOpenGLES)
        : m_attribute(attribute),
          m_oldValue(QCoreApplication::testAttribute(attribute))
    {
        QCoreApplication::setAttribute(attribute, useOpenGLES);
    }

    ~AppAttributeSetter() {
        QCoreApplication::setAttribute(m_attribute, m_oldValue);
    }

private:
    Qt::ApplicationAttribute m_attribute;
    bool m_oldValue = false;
};

struct SurfaceFormatSetter
{
    SurfaceFormatSetter(const QSurfaceFormat &format)
        : m_oldFormat(QSurfaceFormat::defaultFormat())
    {
        QSurfaceFormat::setDefaultFormat(format);
    }

    ~SurfaceFormatSetter() {
        QSurfaceFormat::setDefaultFormat(m_oldFormat);
    }

private:
    QSurfaceFormat m_oldFormat;
};

}

boost::optional<KisOpenGLModeProber::Result>
KisOpenGLModeProber::probeFormat(const QSurfaceFormat &format, bool adjustGlobalState)
{
    QScopedPointer<AppAttributeSetter> sharedContextSetter;
    QScopedPointer<AppAttributeSetter> glSetter;
    QScopedPointer<AppAttributeSetter> glesSetter;
    QScopedPointer<SurfaceFormatSetter> formatSetter;
    QScopedPointer<QApplication> application;

    if (adjustGlobalState) {
        sharedContextSetter.reset(new AppAttributeSetter(Qt::AA_ShareOpenGLContexts, false));

        if (format.renderableType() != QSurfaceFormat::DefaultRenderableType) {
            glSetter.reset(new AppAttributeSetter(Qt::AA_UseDesktopOpenGL, format.renderableType() != QSurfaceFormat::OpenGLES));
            glesSetter.reset(new AppAttributeSetter(Qt::AA_UseOpenGLES, format.renderableType() == QSurfaceFormat::OpenGLES));
        }

        formatSetter.reset(new SurfaceFormatSetter(format));

        int argc = 1;
        QByteArray data("krita");
        char *argv = data.data();
        application.reset(new QApplication(argc, &argv));
    }

    QWindow surface;
    surface.setFormat(format);
    surface.setSurfaceType(QSurface::OpenGLSurface);
    surface.create();
    QOpenGLContext context;
    context.setFormat(format);


    if (!context.create()) {
        dbgOpenGL << "OpenGL context cannot be created";
        return boost::none;
    }
    if (!context.isValid()) {
        dbgOpenGL << "OpenGL context is not valid while checking Qt's OpenGL status";
        return boost::none;
    }
    if (!context.makeCurrent(&surface)) {
        dbgOpenGL << "OpenGL context cannot be made current";
        return boost::none;
    }

    if (!fuzzyCompareColorSpaces(context.format().colorSpace(), format.colorSpace())) {
        dbgOpenGL << "Failed to create an OpenGL context with requested color space. Requested:" << format.colorSpace() << "Actual:" << context.format().colorSpace();
        return boost::none;
    }

    return Result(context);
}

bool KisOpenGLModeProber::fuzzyCompareColorSpaces(const QSurfaceFormat::ColorSpace &lhs, const QSurfaceFormat::ColorSpace &rhs)
{
    return lhs == rhs ||
        ((lhs == QSurfaceFormat::DefaultColorSpace ||
          lhs == QSurfaceFormat::sRGBColorSpace) &&
         (rhs == QSurfaceFormat::DefaultColorSpace ||
          rhs == QSurfaceFormat::sRGBColorSpace));
}

void KisOpenGLModeProber::initSurfaceFormatFromConfig(KisConfig::RootSurfaceFormat config,
                                                      QSurfaceFormat *format)
{
#ifdef HAVE_HDR
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
    } else
#else
    if (config == KisConfig::BT2020_PQ) {
        qWarning() << "WARNING: Bt.2020 PQ surface type is not supoprted by this build of Krita";
    } else if (config == KisConfig::BT709_G10) {
        qWarning() << "WARNING: scRGB surface type is not supoprted by this build of Krita";
    }
#endif

    {
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
#ifdef HAVE_HDR

    bool isBt2020PQ =
        format.colorSpace() == QSurfaceFormat::bt2020PQColorSpace &&
        format.redBufferSize() == 10 &&
        format.greenBufferSize() == 10 &&
        format.blueBufferSize() == 10 &&
        format.alphaBufferSize() == 2;

    bool isBt709G10 =
        format.colorSpace() == QSurfaceFormat::scRGBColorSpace &&
        format.redBufferSize() == 16 &&
        format.greenBufferSize() == 16 &&
        format.blueBufferSize() == 16 &&
        format.alphaBufferSize() == 16;

    return isBt2020PQ || isBt709G10;
#else
    return false;
#endif
}

KisOpenGLModeProber::Result::Result(QOpenGLContext &context) {
    if (!context.isValid()) {
        return;
    }

    QOpenGLFunctions *funcs = context.functions(); // funcs is ready to be used

    m_rendererString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_RENDERER)));
    m_driverVersionString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_VERSION)));
    m_vendorString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_VENDOR)));
    m_shadingLanguageString = QString(reinterpret_cast<const char *>(funcs->glGetString(GL_SHADING_LANGUAGE_VERSION)));
    m_glMajorVersion = context.format().majorVersion();
    m_glMinorVersion = context.format().minorVersion();
    m_supportsDeprecatedFunctions = (context.format().options() & QSurfaceFormat::DeprecatedFunctions);
    m_isOpenGLES = context.isOpenGLES();
    m_format = context.format();
}
