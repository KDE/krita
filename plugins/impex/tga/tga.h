/* This file is part of the KDE project
   Copyright (C) 2003 Dominik Seichter <domseichter@web.de>
   Copyright (C) 2004 Ignacio Castaño <castano@ludicon.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

/* this code supports:
 * reading:
 *     uncompressed and run length encoded indexed, grey and color tga files.
 *     image types 1, 2, 3, 9, 10 and 11.
 *     only RGB color maps with no more than 256 colors.
 *     pixel formats 8, 15, 24 and 32.
 * writing:
 *     uncompressed true color tga files
 */
#ifndef TGA_H
#define TGA_H

#include <QDataStream>
#include <QImage>
#include <QColor>

// Header format of saved files.
const uchar targaMagic[12] = { 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

enum TGAType {
    TGA_TYPE_INDEXED        = 1,
    TGA_TYPE_RGB            = 2,
    TGA_TYPE_GREY           = 3,
    TGA_TYPE_RLE_INDEXED    = 9,
    TGA_TYPE_RLE_RGB        = 10,
    TGA_TYPE_RLE_GREY       = 11
};

#define TGA_INTERLEAVE_MASK 0xc0
#define TGA_INTERLEAVE_NONE 0x00
#define TGA_INTERLEAVE_2WAY 0x40
#define TGA_INTERLEAVE_4WAY 0x80

#define TGA_ORIGIN_MASK     0x30
#define TGA_ORIGIN_LEFT     0x00
#define TGA_ORIGIN_RIGHT    0x10
#define TGA_ORIGIN_LOWER    0x00
#define TGA_ORIGIN_UPPER    0x20

/** Tga Header. */
struct TgaHeader {
    uchar id_length;
    uchar colormap_type;
    uchar image_type;
    ushort colormap_index;
    ushort colormap_length;
    uchar colormap_size;
    ushort x_origin;
    ushort y_origin;
    ushort width;
    ushort height;
    uchar pixel_size;
    uchar flags;

    enum { SIZE = 18 }; // const static int SIZE = 18;
};


struct Color555 {
    ushort b : 5;
    ushort g : 5;
    ushort r : 5;
};

struct TgaHeaderInfo {
    bool rle;
    bool pal;
    bool rgb;
    bool grey;

    TgaHeaderInfo(const TgaHeader & tga) : rle(false), pal(false), rgb(false), grey(false) {
        switch (tga.image_type) {
        case TGA_TYPE_RLE_INDEXED:
            rle = true;
            Q_FALLTHROUGH();
        case TGA_TYPE_INDEXED:
            pal = true;
            break;

        case TGA_TYPE_RLE_RGB:
            rle = true;
            Q_FALLTHROUGH();
        case TGA_TYPE_RGB:
            rgb = true;
            break;
        case TGA_TYPE_RLE_GREY:
            rle = true;
            Q_FALLTHROUGH();
        case TGA_TYPE_GREY:
            grey = true;
            break;
        default:
            // Error, unknown image type.
            break;
        }
    }
};






#endif // TGA_H
