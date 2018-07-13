/*
 *  Copyright (c) 2017 Alvin Wong <alvinhochun@gmail.com>
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

#include "opengl/kis_opengl.h"
#include "opengl/kis_opengl_p.h"

#include <QApplication>
#include <QOpenGLContext>
#include <QRegularExpression>
#include <QStringList>
#include <QWindow>

#include <klocalizedstring.h>

#include <kis_debug.h>
#include <kis_config.h>

#include <boost/optional.hpp>

using namespace KisOpenGLPrivate;

namespace
{

struct WindowsOpenGLStatus {
    bool supportsDesktopGL = false;
    bool supportsAngleD3D11 = false;
    bool isQtPreferAngle = false;
    bool overridePreferAngle = false; // override Qt to force ANGLE to be preferred
};
WindowsOpenGLStatus windowsOpenGLStatus = {};
KisOpenGL::OpenGLRenderer userRendererConfig;
KisOpenGL::OpenGLRenderer nextUserRendererConfig;
KisOpenGL::OpenGLRenderer currentRenderer;

QStringList qpaDetectionLog;

boost::optional<OpenGLCheckResult> checkQpaOpenGLStatus() {
    QWindow surface;
    surface.setSurfaceType(QSurface::OpenGLSurface);
    surface.create();
    QOpenGLContext context;
    if (!context.create()) {
        qDebug() << "OpenGL context cannot be created";
        return boost::none;
    }
    if (!context.isValid()) {
        qDebug() << "OpenGL context is not valid while checking Qt's OpenGL status";
        return boost::none;
    }
    if (!context.makeCurrent(&surface)) {
        qDebug() << "OpenGL context cannot be made current";
        return boost::none;
    }
    return OpenGLCheckResult(context);
}

bool checkIsSupportedDesktopGL(const OpenGLCheckResult &checkResult) {
    if (checkResult.isUsingAngle()) {
        qWarning() << "ANGLE was being used when desktop OpenGL was wanted, assuming no desktop OpenGL support";
        return false;
    }
    if (checkResult.isOpenGLES()) {
        qWarning() << "Got OpenGL ES instead of desktop OpenGL, this shouldn't happen!";
        return false;
    }
    return checkResult.isSupportedVersion();
}

bool checkIsSupportedAngleD3D11(const OpenGLCheckResult &checkResult) {
    if (!checkResult.isOpenGLES()) {
        qWarning() << "Got desktop OpenGL instead of OpenGL ES, this shouldn't happen!";
        return false;
    }
    if (!checkResult.isUsingAngle()) {
        // This can happen if someone tries to swap in SwiftShader, don't mind it.
        qWarning() << "OpenGL ES context is not ANGLE. Continuing anyway...";
    }
    // HACK: Block ANGLE with Direct3D9
    //       Direct3D9 does not give OpenGL ES 3.0
    //       Some versions of ANGLE returns OpenGL version 3.0 incorrectly
    if (checkResult.rendererString().contains("Direct3D9", Qt::CaseInsensitive)) {
        qWarning() << "ANGLE tried to use Direct3D9, Krita won't work with it";
        return false;
    }
    return checkResult.isSupportedVersion();
}

void specialOpenGLVendorFilter(WindowsOpenGLStatus &status, const OpenGLCheckResult &checkResult) {
    if (!status.supportsAngleD3D11) {
        return;
    }

    // Special blacklisting of OpenGL/ANGLE is tracked on:
    // https://phabricator.kde.org/T7411

    // HACK: Specifically detect for Intel driver build number
    //       See https://www.intel.com/content/www/us/en/support/articles/000005654/graphics-drivers.html
    if (checkResult.rendererString().startsWith("Intel")) {
        KLocalizedString knownBadIntelWarning = ki18n("The Intel graphics driver in use is known to have issues with OpenGL.");
        KLocalizedString grossIntelWarning = ki18n(
            "Intel graphics drivers tend to have issues with OpenGL so ANGLE will be used by default. "
            "You may manually switch to OpenGL but it is not guaranteed to work properly."
        );
        QRegularExpression regex("\\b\\d{2}\\.\\d{2}\\.\\d{2}\\.(\\d{4})\\b");
        QRegularExpressionMatch match = regex.match(checkResult.driverVersionString());
        if (match.hasMatch()) {
            int driverBuild = match.captured(1).toInt();
            if (driverBuild > 4636 && driverBuild < 4729) {
                // Make ANGLE the preferred renderer for Intel driver versions
                // between build 4636 and 4729 (exclusive) due to an UI offset bug.
                // See https://communities.intel.com/thread/116003
                // (Build 4636 is known to work from some test results)
                qDebug() << "Detected Intel driver build between 4636 and 4729, making ANGLE the preferred renderer";
                status.overridePreferAngle = true;
                appendOpenGLWarningString(knownBadIntelWarning);
            } else if (driverBuild == 4358) {
                // There are several reports on a bug where the canvas is not being
                // updated properly which has debug info pointing to this build.
                qDebug() << "Detected Intel driver build 4358, making ANGLE the preferred renderer";
                status.overridePreferAngle = true;
                appendOpenGLWarningString(knownBadIntelWarning);
            } else {
                // Intel tends to randomly break OpenGL in some of their new driver
                // builds, therefore we just shouldn't use OpenGL by default to
                // reduce bug report noises.
                qDebug() << "Detected Intel driver, making ANGLE the preferred renderer";
                status.overridePreferAngle = true;
                if (status.supportsDesktopGL) {
                    appendOpenGLWarningString(grossIntelWarning);
                }
            }
        }
    }
}

} // namespace

void KisOpenGLPrivate::appendPlatformOpenGLDebugText(QDebug &debugOut) {
    debugOut << "\n\nQPA OpenGL Detection Info";
    debugOut << "\n  supportsDesktopGL:" << windowsOpenGLStatus.supportsDesktopGL;
    debugOut << "\n  supportsAngleD3D11:" << windowsOpenGLStatus.supportsAngleD3D11;
    debugOut << "\n  isQtPreferAngle:" << windowsOpenGLStatus.isQtPreferAngle;
    debugOut << "\n  overridePreferAngle:" << windowsOpenGLStatus.overridePreferAngle;
    debugOut << "\n== log ==\n";
    debugOut.noquote();
    debugOut << qpaDetectionLog.join('\n');
    debugOut.resetFormat();
    debugOut << "\n== end log ==";
}

/**
 * This function probes the Qt Platform Abstraction (QPA) for OpenGL diagnostics
 * information. The code works under the assumption that the bundled Qt is built
 * with `-opengl dynamic` and includes support for ANGLE.
 *
 * This function is written for Qt 5.9.1. On other versions it might not work
 * as well.
 */
void KisOpenGL::probeWindowsQpaOpenGL(int argc, char **argv, QString userRendererConfigString)
{
    KIS_SAFE_ASSERT_RECOVER(isDefaultFormatSet()) {
        qWarning() << "Default OpenGL format was not set before calling KisOpenGL::probeWindowsQpaOpenGL. This might be a BUG!";
        setDefaultFormat();
    }

    // Clear env var to prevent affecting tests
    qunsetenv("QT_OPENGL");

    boost::optional<OpenGLCheckResult> qpaDetectionResult;

    qDebug() << "Probing Qt OpenGL detection:";
    {
        {
            QGuiApplication app(argc, argv);
            qpaDetectionResult = checkQpaOpenGLStatus();
        }
    }
    if (!qpaDetectionResult) {
        qWarning() << "Could not initialize OpenGL context!";
        return;
    }
    qDebug() << "Done probing Qt OpenGL detection";

    windowsOpenGLStatus.isQtPreferAngle = qpaDetectionResult->isOpenGLES();

    boost::optional<OpenGLCheckResult> checkResultAngle, checkResultDesktopGL;
    if (qpaDetectionResult->isOpenGLES()) {
        checkResultAngle = qpaDetectionResult;
        // We already checked ANGLE, now check desktop OpenGL
        qputenv("QT_OPENGL", "desktop");
        qDebug() << "Checking desktop OpenGL...";
        {
            QGuiApplication app(argc, argv);
            checkResultDesktopGL = checkQpaOpenGLStatus();
        }
        if (!checkResultDesktopGL) {
            qWarning() << "Could not initialize OpenGL context!";
        }
        qDebug() << "Done checking desktop OpenGL";
        qunsetenv("QT_OPENGL");
    } else {
        checkResultDesktopGL = qpaDetectionResult;
        // We already checked desktop OpenGL, now check ANGLE
        qputenv("QT_OPENGL", "angle");
        qDebug() << "Checking ANGLE...";
        {
            QGuiApplication app(argc, argv);
            checkResultAngle = checkQpaOpenGLStatus();
        }
        if (!checkResultAngle) {
            qWarning() << "Could not initialize OpenGL context!";
        }
        qDebug() << "Done checking ANGLE";
        qunsetenv("QT_OPENGL");
    }

    windowsOpenGLStatus.supportsDesktopGL =
            checkResultDesktopGL && checkIsSupportedDesktopGL(*checkResultDesktopGL);
    windowsOpenGLStatus.supportsAngleD3D11 =
            checkResultAngle && checkIsSupportedAngleD3D11(*checkResultAngle);

    if (!windowsOpenGLStatus.supportsDesktopGL) {
        appendOpenGLWarningString(ki18n("The graphics driver in use does not meet the OpenGL requirements."));
    } else if (windowsOpenGLStatus.isQtPreferAngle) {
        appendOpenGLWarningString(ki18n("The graphics driver in use may not work well with OpenGL."));
    }

    // HACK: Filter specific buggy drivers not handled by Qt OpenGL buglist
    if (checkResultDesktopGL) {
        specialOpenGLVendorFilter(windowsOpenGLStatus, *checkResultDesktopGL);
    }

    if (windowsOpenGLStatus.supportsAngleD3D11
            && (checkResultAngle->rendererString().contains("Software Adapter")
                    || checkResultAngle->rendererString().contains("Microsoft Basic Render Driver"))) {
        appendOpenGLWarningString(ki18n(
            "ANGLE is using a software Direct3D renderer, which is not hardware-accelerated and may be very slow. "
            "This can happen if the graphics drivers are not properly installed, or when using a Remote Desktop session."
        ));
    }

    userRendererConfig = convertConfigToOpenGLRenderer(userRendererConfigString);
    if ((userRendererConfig == RendererDesktopGL && !windowsOpenGLStatus.supportsDesktopGL) ||
            (userRendererConfig == RendererAngle && !windowsOpenGLStatus.supportsAngleD3D11)) {
        // Set it to auto so we won't get stuck
        userRendererConfig = RendererAuto;
    }
    nextUserRendererConfig = userRendererConfig;
    switch (userRendererConfig) {
    case RendererDesktopGL:
        QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
        currentRenderer = RendererDesktopGL;
        break;
    case RendererAngle:
        QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);
        currentRenderer = RendererAngle;
        break;
    default:
        if (windowsOpenGLStatus.isQtPreferAngle && windowsOpenGLStatus.supportsAngleD3D11) {
            currentRenderer = RendererAngle;
        } else if (windowsOpenGLStatus.overridePreferAngle && windowsOpenGLStatus.supportsAngleD3D11) {
            QCoreApplication::setAttribute(Qt::AA_UseOpenGLES, true);
            currentRenderer = RendererAngle;
        } else if (!windowsOpenGLStatus.isQtPreferAngle && windowsOpenGLStatus.supportsDesktopGL) {
            currentRenderer = RendererDesktopGL;
        } else {
            currentRenderer = RendererNone;
        }
        break;
    }
}

KisOpenGL::OpenGLRenderer KisOpenGL::getCurrentOpenGLRenderer()
{
    return currentRenderer;
}

KisOpenGL::OpenGLRenderer KisOpenGL::getQtPreferredOpenGLRenderer()
{
    return (windowsOpenGLStatus.isQtPreferAngle || windowsOpenGLStatus.overridePreferAngle)
            ? RendererAngle : RendererDesktopGL;
}

KisOpenGL::OpenGLRenderers KisOpenGL::getSupportedOpenGLRenderers()
{
    return RendererAuto |
            (windowsOpenGLStatus.supportsDesktopGL ? RendererDesktopGL : static_cast<OpenGLRenderer>(0)) |
            (windowsOpenGLStatus.supportsAngleD3D11 ? RendererAngle : static_cast<OpenGLRenderers>(0));
}

KisOpenGL::OpenGLRenderer KisOpenGL::getUserOpenGLRendererConfig()
{
    return userRendererConfig;
}

KisOpenGL::OpenGLRenderer KisOpenGL::getNextUserOpenGLRendererConfig()
{
    return nextUserRendererConfig;
}

void KisOpenGL::setNextUserOpenGLRendererConfig(KisOpenGL::OpenGLRenderer renderer)
{
    nextUserRendererConfig = renderer;
}

QString KisOpenGL::convertOpenGLRendererToConfig(KisOpenGL::OpenGLRenderer renderer)
{
    switch (renderer) {
    case RendererDesktopGL:
        return QStringLiteral("desktop");
    case RendererAngle:
        return QStringLiteral("angle");
    default:
        return QStringLiteral("auto");
    }
}

KisOpenGL::OpenGLRenderer KisOpenGL::convertConfigToOpenGLRenderer(QString renderer)
{
    if (renderer == "desktop") {
        return RendererDesktopGL;
    } else if (renderer == "angle") {
        return RendererAngle;
    } else {
        return RendererAuto;
    }
}
