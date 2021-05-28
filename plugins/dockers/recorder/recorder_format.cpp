/*
 *  SPDX-FileCopyrightText: 2021 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_format.h"

namespace RecorderFormatInfo
{

QLatin1String fileExtension(RecorderFormat format)
{
    switch (format) {
        case RecorderFormat::JPEG:
            return QLatin1String("jpg");
        case RecorderFormat::PNG:
            return QLatin1String("png");
    }

    return QLatin1String("");
}

QLatin1String fileFormat(RecorderFormat format)
{
    switch (format) {
        case RecorderFormat::JPEG:
            return QLatin1String("JPEG");
        case RecorderFormat::PNG:
            return QLatin1String("PNG");
    }

    return QLatin1String("");
}

}
