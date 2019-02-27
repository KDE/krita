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
#ifndef KIS_OPENGL_P_H_
#define KIS_OPENGL_P_H_

#include <QtGlobal>
#include <QString>

class QDebug;
class QOpenGLContext;

class KLocalizedString;

namespace KisOpenGLPrivate
{

class OpenGLCheckResult {
    int m_glMajorVersion = 0;
    int m_glMinorVersion = 0;
    bool m_supportsDeprecatedFunctions = false;
    bool m_isOpenGLES = false;
    QString m_rendererString;
    QString m_driverVersionString;

public:
    OpenGLCheckResult(QOpenGLContext &context);

    int glMajorVersion() const {
        return m_glMajorVersion;
    }

    int glMinorVersion() const {
        return m_glMinorVersion;
    }

    bool supportsDeprecatedFunctions() const {
        return m_supportsDeprecatedFunctions;
    }

    bool isOpenGLES() const {
        return m_isOpenGLES;
    }

    QString rendererString() const {
        return m_rendererString;
    }

    QString driverVersionString() const {
        return m_driverVersionString;
    }

    bool isSupportedVersion() const {
        return
#ifdef Q_OS_OSX
                ((m_glMajorVersion * 100 + m_glMinorVersion) >= 302)
#else
                (m_glMajorVersion >= 3 && (m_supportsDeprecatedFunctions || m_isOpenGLES)) ||
                ((m_glMajorVersion * 100 + m_glMinorVersion) == 201)
#endif
                ;
    }

    bool supportsLoD() const {
        return (m_glMajorVersion * 100 + m_glMinorVersion) >= 300;
    }

    bool hasOpenGL3() const {
        return (m_glMajorVersion * 100 + m_glMinorVersion) >= 302;
    }

    bool supportsFenceSync() const {
        return m_glMajorVersion >= 3;
    }

#ifdef Q_OS_WIN
    // This is only for detecting whether ANGLE is being used.
    // For detecting generic OpenGL ES please check isOpenGLES
    bool isUsingAngle() const {
        return m_rendererString.startsWith("ANGLE", Qt::CaseInsensitive);
    }
#endif
};

void appendPlatformOpenGLDebugText(QDebug &debugOut);
#ifndef Q_OS_WIN
void appendPlatformOpenGLDebugText(QDebug &/*debugOut*/) {}
#endif

void appendOpenGLWarningString(KLocalizedString warning);
void overrideOpenGLWarningString(QVector<KLocalizedString> warnings);

bool isDefaultFormatSet();

} // namespace KisOpenGLPrivate

#endif // KIS_OPENGL_P_H_
