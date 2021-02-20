/*
 *  SPDX-FileCopyrightText: 2009 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef _KIS_JPEG_DESTINATION_H_
#define _KIS_JPEG_DESTINATION_H_

#include <stdio.h>
#include <jpeglib.h>

class QIODevice;

namespace KisJPEGDestination
{
    void setDestination(j_compress_ptr cinfo, QIODevice* destinationDevice);
}

#endif
