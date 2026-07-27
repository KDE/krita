/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef KISLIBAVENCODERCONTEXT
#define KISLIBAVENCODERCONTEXT

#include <QImage>
#include <QString>

#include <klocalizedstring.h>

#include <kis_debug.h>

extern "C" {
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libswscale/swscale.h>
}

class KisLibavEncoderContext
{
    Q_DISABLE_COPY_MOVE(KisLibavEncoderContext)
public:
    explicit KisLibavEncoderContext(QString *outErrorMessage = nullptr)
        : m_outErrorMessage(outErrorMessage)
    {
    }

    virtual ~KisLibavEncoderContext()
    {
        sws_freeContext(m_swsContext);
    };

    bool convertFrame(QImage &inOutImage, AVPixelFormat &outPixelFormat) const
    {
        switch (inOutImage.format()) {
        case QImage::Format_RGB32:
            outPixelFormat = AV_PIX_FMT_BGR0;
            break;
        case QImage::Format_ARGB32:
            outPixelFormat = AV_PIX_FMT_BGRA;
            break;
        default:
            // The above are the only formats I can get the the recorder to
            // produce, so I'm not gonna get experimental with this.
            outPixelFormat = AV_PIX_FMT_BGRA;
            inOutImage = inOutImage.convertToFormat(QImage::Format_ARGB32);
            if (inOutImage.isNull()) {
                warnFile << "Frame conversion from" << inOutImage.format() << "failed";
                return false;
            }
            break;
        }
        return true;
    }

    int getSwsFlags(const QString &scaleFilter) const
    {
        // These are the values provided by KisDlgAnimationRenderer.
        if (scaleFilter.isEmpty() || scaleFilter == QStringLiteral("bilinear")) {
            return SWS_FAST_BILINEAR;
        } else if (scaleFilter == QStringLiteral("bicubic")) {
            return SWS_BICUBIC;
        } else if (scaleFilter == QStringLiteral("lanczos")) {
            return SWS_LANCZOS;
        } else if (scaleFilter == QStringLiteral("neighbor")) {
            return 0;
        } else if (scaleFilter == QStringLiteral("spline")) {
            return SWS_SPLINE;
        } else {
            warnFile.nospace() << "Unhandled scale filter '" << scaleFilter << "'";
            return SWS_FAST_BILINEAR;
        }
    }

    SwsContext *getSwsContextFor(int inputWidth,
                                 int inputHeight,
                                 AVPixelFormat inputFormat,
                                 int outputWidth,
                                 int outputHeight,
                                 AVPixelFormat outputFormat,
                                 int flags)
    {
        return sws_getCachedContext(m_swsContext,
                                    inputWidth,
                                    inputHeight,
                                    inputFormat,
                                    outputWidth,
                                    outputHeight,
                                    outputFormat,
                                    flags,
                                    nullptr,
                                    nullptr,
                                    nullptr);
    }

    void setInternalErrorMessage(const QString &detail)
    {
        warnFile << "Media encoder error:" << detail;
        // Internal encoder errors are only really useful for developers,
        // so there's no point in translating them.
        setErrorMessage(i18n("Internal error (%1)", detail));
    }

    void setErrorMessage(const QString &errorMessage)
    {
        if (m_outErrorMessage) {
            *m_outErrorMessage = errorMessage;
        }
    }

private:
    QString *m_outErrorMessage;
    SwsContext *m_swsContext = nullptr;
};

#endif
