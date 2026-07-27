/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */
#ifndef KISLIBAVDMEDIAENCODERRUNNABLE
#define KISLIBAVDMEDIAENCODERRUNNABLE

#include "KisMediaEncoderWrapper.h"
#include <QWidget>

class KisLibavMediaEncoderRunnable : public KisMediaEncoderRunnable
{
public:
    static KisLibavMediaEncoderRunnable *create(const KisMediaEncoderWrapperSettings &settings,
                                                QObject *parent = nullptr);

    static void getSupportedFormats(QVector<KisMediaEncoderFormat *> &outSupportedFormats);

protected:
    KisLibavMediaEncoderRunnable(const KisMediaEncoderWrapperSettings &settings, QObject *parent);

    EncodeResult encode(QString &outErrorMessage) override;

private:
    static constexpr int FORMAT_GIF_PALETTE = 0;
    static constexpr int FORMAT_GIF = 1;
    static constexpr int FORMAT_WEBP = 2;
    static constexpr int FORMAT_APNG = 3;

    class Format;
    class Context;
};

#endif
