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


#ifndef KISOPENGLMODEPROBER_H
#define KISOPENGLMODEPROBER_H

#include "kritaui_export.h"
#include "kis_config.h"
#include <QSurfaceFormat>
#include <boost/optional.hpp>

class KoColorProfile;

class KRITAUI_EXPORT KisOpenGLModeProber
{
public:
    class Result;

public:
    KisOpenGLModeProber();
    ~KisOpenGLModeProber();

    static KisOpenGLModeProber* instance();

    bool useHDRMode() const;
    QSurfaceFormat surfaceformatInUse() const;

    const KoColorProfile *rootSurfaceColorProfile() const;

    boost::optional<Result> probeFormat(const QSurfaceFormat &format,
                                        bool adjustGlobalState = true);

    static bool fuzzyCompareColorSpaces(const QSurfaceFormat::ColorSpace &lhs,
                                        const QSurfaceFormat::ColorSpace &rhs);

public:
    static void initSurfaceFormatFromConfig(KisConfig::RootSurfaceFormat config,
                                            QSurfaceFormat *format);
    static bool isFormatHDR(const QSurfaceFormat &format);
};

class KisOpenGLModeProber::Result {
public:
    Result(QOpenGLContext &context);

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

    QString shadingLanguageString() const
    {
        return m_shadingLanguageString;
    }

    QString vendorString() const
    {
        return m_vendorString;
    }

    QSurfaceFormat format() const
    {
        return m_format;
    }

private:
    int m_glMajorVersion = 0;
    int m_glMinorVersion = 0;
    bool m_supportsDeprecatedFunctions = false;
    bool m_isOpenGLES = false;
    QString m_rendererString;
    QString m_driverVersionString;
    QString m_vendorString;
    QString m_shadingLanguageString;
    QSurfaceFormat m_format;
};

#endif // KISOPENGLMODEPROBER_H
