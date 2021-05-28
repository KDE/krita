/*
 *  SPDX-FileCopyrightText: 2021 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDER_FORMAT_H
#define RECORDER_FORMAT_H

#include <QLatin1String>

enum class RecorderFormat
{
    JPEG,
    PNG
};

namespace RecorderFormatInfo
{

QLatin1String fileExtension(RecorderFormat format);

QLatin1String fileFormat(RecorderFormat format);

}

#endif // RECORDER_FORMAT_H
