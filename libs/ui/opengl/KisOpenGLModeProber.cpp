/*
 *  SPDX-FileCopyrightText: 2017 Alvin Wong <alvinhochun@gmail.com>
 *  SPDX-FileCopyrightText: 2019 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisOpenGLModeProber.h"

#include <config-hdr.h>
#include <QApplication>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QWindow>
#include <QColorSpace>

#include <QGlobalStatic>
#include <KisSurfaceColorSpaceWrapper.h>

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
    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();

    const auto surfaceColorSpace = 
        KisSurfaceColorSpaceWrapper::fromQtColorSpace(surfaceformatInUse().colorSpace());
    
    if (surfaceColorSpace == KisSurfaceColorSpaceWrapper::sRGBColorSpace) {
        // use the default one!
#ifdef HAVE_HDR
    } else if (surfaceColorSpace == KisSurfaceColorSpaceWrapper::scRGBColorSpace) {
        profile = KoColorSpaceRegistry::instance()->p709G10Profile();
    } else if (surfaceColorSpace == KisSurfaceColorSpaceWrapper::bt2020PQColorSpace) {
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


struct EnvironmentSetter
{
    EnvironmentSetter(const QLatin1String &env, const QString &value)
        : m_env(env)
    {
        if (!qEnvironmentVariableIsEmpty(m_env.latin1())) {
            m_oldValue = qgetenv(env.latin1());
        }
        if (!value.isEmpty()) {
            qputenv(env.latin1(), value.toLatin1());
        } else {
            qunsetenv(env.latin1());
        }
    }

    ~EnvironmentSetter() {
        if (m_oldValue) {
            qputenv(m_env.latin1(), (*m_oldValue).toLatin1());
        } else {
            qunsetenv(m_env.latin1());
        }
    }

private:
    const QLatin1String m_env;
    boost::optional<QString> m_oldValue;
};

}

boost::optional<KisOpenGLModeProber::Result>
KisOpenGLModeProber::probeFormat(const KisOpenGL::RendererConfig &rendererConfig,
                                 bool adjustGlobalState)
{
    const QSurfaceFormat &format = rendererConfig.format;

    dbgOpenGL << "Probing format" << rendererConfig.rendererId() << rendererConfig.angleRenderer
              << rendererConfig.format;

    QScopedPointer<AppAttributeSetter> sharedContextSetter;
    QScopedPointer<AppAttributeSetter> glSetter;
    QScopedPointer<AppAttributeSetter> glesSetter;
    QScopedPointer<SurfaceFormatSetter> formatSetter;
    QScopedPointer<EnvironmentSetter> rendererSetter;
    QScopedPointer<EnvironmentSetter> portalSetter;
    QScopedPointer<QGuiApplication> application;

    int argc = 1;
    QByteArray probeAppName("krita");
    char *argv = probeAppName.data();


    if (adjustGlobalState) {
        sharedContextSetter.reset(new AppAttributeSetter(Qt::AA_ShareOpenGLContexts, false));

        if (format.renderableType() != QSurfaceFormat::DefaultRenderableType) {
            glSetter.reset(new AppAttributeSetter(Qt::AA_UseDesktopOpenGL, format.renderableType() != QSurfaceFormat::OpenGLES));
            glesSetter.reset(new AppAttributeSetter(Qt::AA_UseOpenGLES, format.renderableType() == QSurfaceFormat::OpenGLES));
        }

        if (!qEnvironmentVariableIsSet("QT_ANGLE_PLATFORM")) {
            rendererSetter.reset(new EnvironmentSetter(QLatin1String("QT_ANGLE_PLATFORM"), angleRendererToString(rendererConfig.angleRenderer)));
        }
        portalSetter.reset(new EnvironmentSetter(QLatin1String("QT_NO_XDG_DESKTOP_PORTAL"), QLatin1String("1")));
        formatSetter.reset(new SurfaceFormatSetter(format));

        // Disable this workaround for plasma (BUG:408015), because it causes 
        // a crash on Windows with Qt 5.15.7
        const bool runningInKDE =  qEnvironmentVariableIsSet("KDE_FULL_SESSION");
        const bool isInAppimage = qEnvironmentVariableIsSet("APPIMAGE");

        if (runningInKDE && !isInAppimage) {
            QGuiApplication::setDesktopSettingsAware(false);
        }

        application.reset(new QGuiApplication(argc, &argv));

        if (runningInKDE && !isInAppimage) {
            QGuiApplication::setDesktopSettingsAware(true);
        }

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

    if (!fuzzyCompareColorSpaces(
            KisSurfaceColorSpaceWrapper::fromQtColorSpace(context.format().colorSpace()),
            KisSurfaceColorSpaceWrapper::fromQtColorSpace(format.colorSpace()))) {

        dbgOpenGL << "Failed to create an OpenGL context with requested color space. Requested:" << format.colorSpace() << "Actual:" << context.format().colorSpace();
        return boost::none;
    }

    if (format.redBufferSize() > 0 && format.greenBufferSize() > 0 && format.blueBufferSize() > 0
        && (context.format().redBufferSize() != format.redBufferSize()
            || context.format().greenBufferSize() != format.greenBufferSize()
            || context.format().blueBufferSize() != format.blueBufferSize())) {

        dbgOpenGL << "Failed to create an OpenGL context with requested bit depth. Requested:" << format.redBufferSize()
                  << "Actual:" << context.format().redBufferSize();
        return boost::none;
    }

    Result result(context);

    dbgOpenGL << "Probe returned" << result.rendererString() << result.driverVersionString() << result.isOpenGLES();

    return result;
}

bool KisOpenGLModeProber::fuzzyCompareColorSpaces(const KisSurfaceColorSpaceWrapper &lhs, const KisSurfaceColorSpaceWrapper &rhs)
{
    return lhs == rhs ||
        ((lhs == KisSurfaceColorSpaceWrapper::DefaultColorSpace ||
          lhs == KisSurfaceColorSpaceWrapper::sRGBColorSpace) &&
         (rhs == KisSurfaceColorSpaceWrapper::DefaultColorSpace ||
          rhs == KisSurfaceColorSpaceWrapper::sRGBColorSpace));
}

void KisOpenGLModeProber::initSurfaceFormatFromConfig(std::pair<KisSurfaceColorSpaceWrapper, int> rootSurfaceFormat,
                                                      QSurfaceFormat *format)
{
#ifdef HAVE_HDR
    /**
     * In Windows' HDR we have three fixed modes only.
     */
    if (rootSurfaceFormat.first == KisSurfaceColorSpaceWrapper::bt2020PQColorSpace) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(rootSurfaceFormat.second == 10);
        format->setRedBufferSize(10);
        format->setGreenBufferSize(10);
        format->setBlueBufferSize(10);
        format->setAlphaBufferSize(2);
        format->setColorSpace(KisSurfaceColorSpaceWrapper(KisSurfaceColorSpaceWrapper::bt2020PQColorSpace));
    } else if (rootSurfaceFormat.first == KisSurfaceColorSpaceWrapper::scRGBColorSpace) {
        KIS_SAFE_ASSERT_RECOVER_NOOP(rootSurfaceFormat.second == 16);
        format->setRedBufferSize(16);
        format->setGreenBufferSize(16);
        format->setBlueBufferSize(16);
        format->setAlphaBufferSize(16);
        format->setColorSpace(KisSurfaceColorSpaceWrapper(KisSurfaceColorSpaceWrapper::scRGBColorSpace));
    } else {
        KIS_SAFE_ASSERT_RECOVER_NOOP(rootSurfaceFormat.second == 8);
        format->setRedBufferSize(8);
        format->setGreenBufferSize(8);
        format->setBlueBufferSize(8);
        format->setAlphaBufferSize(8);
        // TODO: check if we can use real sRGB space here
        format->setColorSpace(KisSurfaceColorSpaceWrapper());
    }
#else
    /**
     * In Wayland's HDR we don't set the mode on the root surface, we should only select
     * bit-width.
     */
    if (rootSurfaceFormat.first == KisSurfaceColorSpaceWrapper::bt2020PQColorSpace) {
        qWarning() << "WARNING: Bt.2020 PQ surface type is not supported by this build of Krita";
        rootSurfaceFormat.first = KisSurfaceColorSpaceWrapper::DefaultColorSpace;
    } else if (rootSurfaceFormat.first == KisSurfaceColorSpaceWrapper::scRGBColorSpace) {
        qWarning() << "WARNING: scRGB surface type is not supported by this build of Krita";
        rootSurfaceFormat.first = KisSurfaceColorSpaceWrapper::DefaultColorSpace;
    } else {
        KIS_SAFE_ASSERT_RECOVER_NOOP(rootSurfaceFormat.first == KisSurfaceColorSpaceWrapper::DefaultColorSpace);
        KIS_SAFE_ASSERT_RECOVER_NOOP(rootSurfaceFormat.second == 8 || rootSurfaceFormat.second == 10);

        if (rootSurfaceFormat.second == 10) {
            format->setRedBufferSize(10);
            format->setGreenBufferSize(10);
            format->setBlueBufferSize(10);
            format->setAlphaBufferSize(2);
            // TODO: check if we can use real sRGB space here
            format->setColorSpace(KisSurfaceColorSpaceWrapper());
        } else {
            format->setRedBufferSize(8);
            format->setGreenBufferSize(8);
            format->setBlueBufferSize(8);
            format->setAlphaBufferSize(8);
            // TODO: check if we can use real sRGB space here
            format->setColorSpace(KisSurfaceColorSpaceWrapper());
        }
    }
#endif
}

bool KisOpenGLModeProber::isFormatHDR(const QSurfaceFormat &format)
{
#ifdef HAVE_HDR

    bool isBt2020PQ =
        format.colorSpace() == KisSurfaceColorSpaceWrapper::makeBt2020PQColorSpace() &&
        format.redBufferSize() == 10 &&
        format.greenBufferSize() == 10 &&
        format.blueBufferSize() == 10 &&
        format.alphaBufferSize() == 2;

    bool isBt709G10 =
        format.colorSpace() == KisSurfaceColorSpaceWrapper::makeSCRGBColorSpace() &&
        format.redBufferSize() == 16 &&
        format.greenBufferSize() == 16 &&
        format.blueBufferSize() == 16 &&
        format.alphaBufferSize() == 16;

    return isBt2020PQ || isBt709G10;
#else
    Q_UNUSED(format);
    return false;
#endif
}

QString KisOpenGLModeProber::angleRendererToString(KisOpenGL::AngleRenderer renderer)
{
    QString value;

    switch (renderer) {
    case KisOpenGL::AngleRendererDefault:
        break;
    case KisOpenGL::AngleRendererD3d9:
        value = "d3d9";
        break;
    case KisOpenGL::AngleRendererD3d11:
        value = "d3d11";
        break;
    case KisOpenGL::AngleRendererD3d11Warp:
        value = "warp";
        break;
    };

    return value;
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
    m_supportsFBO = context.functions()->hasOpenGLFeature(QOpenGLFunctions::Framebuffers);

    m_supportsBufferMapping = !m_isOpenGLES ||
            m_glMajorVersion >= 3 ||
            context.hasExtension("GL_OES_mapbuffer") ||
            context.hasExtension("GL_EXT_map_buffer_range") ||
            context.hasExtension("GL_ARB_map_buffer_range");

    m_supportsBufferInvalidation = !m_isOpenGLES &&
            ((m_glMajorVersion >= 4 && m_glMinorVersion >= 3) ||
             context.hasExtension("GL_ARB_invalidate_subdata"));
    m_supportsLod = context.format().majorVersion() >= 3 || (m_isOpenGLES && context.hasExtension("GL_EXT_shader_texture_lod"));

    m_extensions = context.extensions();
    // Remove empty name extension that sometimes appears on NVIDIA output
    m_extensions.remove("");
}
