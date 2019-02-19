/*
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

#include "KisScreenInformationAdapter.h"

#include "kis_debug.h"
#include <QOpenGLContext>

#include <QGuiApplication>
#include <QWindow>

#include <config-hdr.h>

#ifdef Q_OS_WIN
#if (QT_VERSION == QT_VERSION_CHECK(5, 11, 2))
#include <QtGui/5.11.2/QtGui/qpa/qplatformnativeinterface.h>
#elif (QT_VERSION == QT_VERSION_CHECK(5, 12, 0))
#include <QtGui/5.12.0/QtGui/qpa/qplatformnativeinterface.h>
#elif (QT_VERSION == QT_VERSION_CHECK(5, 12, 1))
#include <QtGui/5.12.1/QtGui/qpa/qplatformnativeinterface.h>
#elif (QT_VERSION == QT_VERSION_CHECK(5, 12, 2))
#include <QtGui/5.12.2/QtGui/qpa/qplatformnativeinterface.h>
#elif (QT_VERSION == QT_VERSION_CHECK(5, 13, 0))
#include <QtGui/5.13.0/QtGui/qpa/qplatformnativeinterface.h>
#endif

#include <d3d11.h>
#include <wrl/client.h>
#include <dxgi1_6.h>
#include "EGL/egl.h"
#include "EGL/eglext.h"
#endif

namespace {
struct EGLException {
    EGLException() {}
    EGLException(const QString &what) : m_what(what) {}

    QString what() const {
        return m_what;
    }

private:
    QString m_what;
};

template <typename FuncType>
void getProcAddressSafe(QOpenGLContext *context, const char *funcName, FuncType &func)
{
    func = reinterpret_cast<FuncType>(context->getProcAddress(funcName));
    if (!func) {
        throw EGLException(QString("failed to fetch function %1").arg(funcName));
    }
}

#ifdef Q_OS_WIN
typedef const char *(EGLAPIENTRYP PFNEGLQUERYSTRINGPROC) (EGLDisplay dpy, EGLint name);
#endif
}


struct KisScreenInformationAdapter::Private
{
    void initialize(QOpenGLContext *context);

    QOpenGLContext *context;
    QString errorString;

#ifdef Q_OS_WIN
    Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter;
#endif
};

KisScreenInformationAdapter::KisScreenInformationAdapter(QOpenGLContext *context)
    : m_d(new Private)
{
    m_d->initialize(context);
}

KisScreenInformationAdapter::~KisScreenInformationAdapter()
{
}

void KisScreenInformationAdapter::Private::initialize(QOpenGLContext *newContext)
{
    context = newContext;
    errorString.clear();

    try {

#ifdef Q_OS_WIN

        if (!context->isOpenGLES()) {
            throw EGLException("the context is not OpenGL ES");
        }

        PFNEGLQUERYSTRINGPROC queryString = nullptr;
        getProcAddressSafe(context, "eglQueryString", queryString);

        const char* client_extensions = queryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
        const QList<QByteArray> extensions = QByteArray(client_extensions).split(' ');

        if (!extensions.contains("EGL_ANGLE_platform_angle_d3d") ||
            !extensions.contains("EGL_ANGLE_device_creation_d3d11")) {

            throw EGLException("the context is not Angle + D3D11");
        }

        PFNEGLQUERYDISPLAYATTRIBEXTPROC queryDisplayAttribEXT = nullptr;
        PFNEGLQUERYDEVICEATTRIBEXTPROC queryDeviceAttribEXT = nullptr;

        getProcAddressSafe(context, "eglQueryDisplayAttribEXT", queryDisplayAttribEXT);
        getProcAddressSafe(context, "eglQueryDeviceAttribEXT", queryDeviceAttribEXT);

        QPlatformNativeInterface *nativeInterface = qGuiApp->platformNativeInterface();
        EGLDisplay display = reinterpret_cast<EGLDisplay>(nativeInterface->nativeResourceForContext("egldisplay", context));

        if (!display) {
            throw EGLException(
                QString("couldn't request EGLDisplay handle, display = 0x%1").arg(uintptr_t(display), 0, 16));
        }

        EGLAttrib value = 0;
        EGLBoolean result = false;

        result = queryDisplayAttribEXT(display, EGL_DEVICE_EXT, &value);

        if (!result || value == EGL_NONE) {
            throw EGLException(
               QString("couldn't request EGLDeviceEXT handle, result = 0x%1, value = 0x%2")
                   .arg(result, 0, 16).arg(value, 0, 16));
        }

        EGLDeviceEXT device = reinterpret_cast<EGLDeviceEXT>(value);

        result = queryDeviceAttribEXT(device, EGL_D3D11_DEVICE_ANGLE, &value);

        if (!result || value == EGL_NONE) {
            throw EGLException(
                QString("couldn't request ID3D11Device pointer, result = 0x%1, value = 0x%2")
                    .arg(result, 0, 16).arg(value, 0, 16));
        }
        ID3D11Device *deviceD3D = reinterpret_cast<ID3D11Device*>(value);

        {
            HRESULT result = 0;

            Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
            result = deviceD3D->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);

            if (FAILED(result)) {
                throw EGLException(
                    QString("couldn't request IDXGIDevice pointer, result = 0x%1").arg(result, 0, 16));
            }

            Microsoft::WRL::ComPtr<IDXGIAdapter1> dxgiAdapter;
            result = dxgiDevice->GetParent(__uuidof(IDXGIAdapter1), (void**)&dxgiAdapter);

            if (FAILED(result)) {
                throw EGLException(
                    QString("couldn't request IDXGIAdapter1 pointer, result = 0x%1").arg(result, 0, 16));
            }

            this->dxgiAdapter = dxgiAdapter;
        }

#else
        throw EGLException("current platform doesn't support fetching display information");
#endif

    } catch (EGLException &e) {
        this->context = 0;
        this->errorString = e.what();
#ifdef Q_OS_WIN
        this->dxgiAdapter.Reset();
#endif
    }
}

bool KisScreenInformationAdapter::isValid() const
{
#ifdef Q_OS_WIN
    return m_d->context && m_d->dxgiAdapter;
#else
    return false;
#endif
}

QString KisScreenInformationAdapter::errorString() const
{
    return m_d->errorString;
}

KisScreenInformationAdapter::ScreenInfo KisScreenInformationAdapter::infoForScreen(QScreen *screen) const
{
    ScreenInfo info;

#ifdef Q_OS_WIN

    QPlatformNativeInterface *nativeInterface = qGuiApp->platformNativeInterface();
    HMONITOR monitor = reinterpret_cast<HMONITOR>(nativeInterface->nativeResourceForScreen("handle", screen));

    if (!monitor) {
        qWarning("%s: failed to get HMONITOR handle for screen: screen = 0x%X, monitor = 0x%X",
                 __PRETTY_FUNCTION__, screen, monitor);
    }

    UINT i = 0;
    Microsoft::WRL::ComPtr<IDXGIOutput> currentOutput;

    while (m_d->dxgiAdapter->EnumOutputs(i, &currentOutput) != DXGI_ERROR_NOT_FOUND)
    {

        HRESULT result = 0;
        Microsoft::WRL::ComPtr<IDXGIOutput6> output6;
        result = currentOutput.As(&output6);

        if (output6) {
            DXGI_OUTPUT_DESC1 desc;
            result = output6->GetDesc1(&desc);

            if (desc.Monitor == monitor) {
                info.screen = screen;
                info.bitsPerColor = desc.BitsPerColor;
                info.redPrimary[0] = desc.RedPrimary[0];
                info.redPrimary[1] = desc.RedPrimary[1];
                info.greenPrimary[0] = desc.GreenPrimary[0];
                info.greenPrimary[1] = desc.GreenPrimary[1];
                info.bluePrimary[0] = desc.BluePrimary[0];
                info.bluePrimary[1] = desc.BluePrimary[1];
                info.whitePoint[0] = desc.WhitePoint[0];
                info.whitePoint[1] = desc.WhitePoint[1];
                info.minLuminance = desc.MinLuminance;
                info.maxLuminance = desc.MaxLuminance;
                info.maxFullFrameLuminance = desc.MaxFullFrameLuminance;

                info.colorSpace = QSurfaceFormat::DefaultColorSpace;

                if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709) {
                    info.colorSpace = QSurfaceFormat::sRGBColorSpace;
                } else if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G22_NONE_P709) {
#ifdef HAVE_HDR
                    info.colorSpace = QSurfaceFormat::scRGBColorSpace;
#else
                    qWarning("WARNING: scRGB display color space is not supported by Qt's build");
#endif
                } else if (desc.ColorSpace == DXGI_COLOR_SPACE_RGB_FULL_G2084_NONE_P2020) {
#ifdef HAVE_HDR
                    info.colorSpace = QSurfaceFormat::bt2020PQColorSpace;
#else
                    qWarning("WARNING: bt2020-pq display color space is not supported by Qt's build");
#endif
                } else {
                    qWarning("WARNING: unknown display color space! 0x%X", desc.ColorSpace);
                }

                break;
            }
        }

        i++;
    }

#endif
    Q_UNUSED(screen);
    return info;
}

QDebug operator<<(QDebug dbg, const KisScreenInformationAdapter::ScreenInfo &info)
{
    QDebugStateSaver saver(dbg);

    if (info.isValid()) {
        dbg.nospace() << "ScreenInfo("
                      << "screen " << info.screen
                      << ", bitsPerColor " << info.bitsPerColor
                      << ", colorSpace " << info.colorSpace
                      << ", redPrimary " << "(" << info.redPrimary[0] << ", " << info.redPrimary[1] << ")"
                      << ", greenPrimary " << "(" << info.greenPrimary[0] << ", " << info.greenPrimary[1] << ")"
                      << ", bluePrimary " << "(" << info.bluePrimary[0] << ", " << info.bluePrimary[1] << ")"
                      << ", whitePoint " << "(" << info.whitePoint[0] << ", " << info.whitePoint[1] << ")"
                      << ", minLuminance " << info.minLuminance
                      << ", maxLuminance " << info.maxLuminance
                      << ", maxFullFrameLuminance " << info.maxFullFrameLuminance
                      << ')';
    } else {
        dbg.nospace() << "ScreenInfo(<invalid>)";
    }

    return dbg;
}
