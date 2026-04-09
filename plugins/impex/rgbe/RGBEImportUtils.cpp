/*
 * SPDX-FileCopyrightText: 2023 Rasyuqa A. H. <qampidh@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Based on KImageFormats Radiance HDR loader
 *
 * SPDX-FileCopyrightText: 2005 Christoph Hormann <chris_hormann@gmx.de>
 * SPDX-FileCopyrightText: 2005 Ignacio Castaño <castanyo@yahoo.es>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <QDataStream>
#include <QIODevice>
#include <kis_sequential_iterator.h>

namespace RGBEIMPORT
{
#define MINELEN 8 // minimum scanline length for encoding
#define MAXELEN 0x7fff // maximum scanline length for encoding

// read an old style line from the hdr image file
// if 'first' is true the first byte is already read
bool ReadOldLine(quint8 *image, int width, QDataStream &s)
{
    int rshift = 0;
    int i;

    while (width > 0) {
        s >> image[0];
        s >> image[1];
        s >> image[2];
        s >> image[3];

        if (s.atEnd()) {
            return false;
        }

        if ((image[0] == 1) && (image[1] == 1) && (image[2] == 1)) {
            const int length = image[3] << rshift;
            if (length > width) {
                dbgFile << "Broken file detected: cannot duplicate pixels past image bounds!";
                return false;
            }
            for (i = length; i > 0; i--) {
                memcpy(image, image-4, 4);
                image += 4;
                width--;
            }
            rshift += 8;
        } else {
            image += 4;
            width--;
            rshift = 0;
        }
    }
    return true;
}

void RGBEToPaintDevice(quint8 *image, int width, KisSequentialIterator &it)
{
    for (int j = 0; j < width; j++) {
        it.nextPixel();
        auto *dst = reinterpret_cast<float *>(it.rawData());

        if (image[3]) {
            const float v = std::ldexp(1.0f, int(image[3]) - (128 + 8));
            const float pixelData[4] = {float(image[0]) * v,
                                        float(image[1]) * v,
                                        float(image[2]) * v,
                                        1.0f};
            memcpy(dst, pixelData, 4 * sizeof(float));
        } else {
            // Zero exponent handle
            const float pixelData[4] = {0.0f, 0.0f, 0.0f, 1.0f};
            memcpy(dst, pixelData, 4 * sizeof(float));
        }

        image += 4;
    }
}

// Load the HDR image.
bool LoadHDR(QDataStream &s, const int width, const int height, KisSequentialIterator &it)
{
    quint8 val;
    quint8 code;

    QByteArray lineArray;
    lineArray.resize(4 * width);
    quint8 *image = (quint8 *)lineArray.data();

    for (int cline = 0; cline < height; cline++) {
        // determine scanline type
        if ((width < MINELEN) || (MAXELEN < width)) {
            if (!ReadOldLine(image, width, s)) {
                return false;
            }
            RGBEToPaintDevice(image, width, it);
            continue;
        }

        s >> val;

        if (s.atEnd()) {
            return true;
        }

        if (val != 2) {
            s.device()->ungetChar(val);
            if (!ReadOldLine(image, width, s)) {
                return false;
            }
            RGBEToPaintDevice(image, width, it);
            continue;
        }

        s >> image[1];
        s >> image[2];
        s >> image[3];

        if (s.atEnd()) {
            return true;
        }

        if ((image[1] != 2) || (image[2] & 128)) {
            image[0] = 2;
            if (!ReadOldLine(image + 4, width - 1, s)) {
                return false;
            }
            RGBEToPaintDevice(image, width, it);
            continue;
        }

        if ((image[2] << 8 | image[3]) != width) {
            dbgFile << "Line of pixels had width" << (image[2] << 8 | image[3]) << "instead of" << width;
            return false;
        }

        // read each component
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < width;) {
                s >> code;
                if (s.atEnd()) {
                    dbgFile << "Truncated HDR file";
                    return false;
                }
                if (code > 128) {
                    // run
                    code &= 127;
                    s >> val;
                    if (j + code - 1 >= width) {
                        dbgFile << "Broken file detected: cannot duplicate data past image bounds!";
                        return false;
                    }
                    while (code != 0) {
                        image[i + j * 4] = val;
                        j++;
                        code--;
                    }
                } else {
                    // non-run
                    if (j + code - 1 >= width) {
                        dbgFile << "Broken file detected: cannot extract data past image bounds!";
                        return false;
                    }
                    while (code != 0) {
                        s >> image[i + j * 4];
                        j++;
                        code--;
                    }
                }
            }
        }

        RGBEToPaintDevice(image, width, it);
    }

    return true;
}

} // namespace RGBEIMPORT
