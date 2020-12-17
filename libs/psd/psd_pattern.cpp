/*
 *  SPDX-FileCopyrightText: 2014 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "psd_pattern.h"

#include "psd_utils.h"
#include "compression.h"

#include <QImage>
#include <kis_debug.h>

struct PsdPattern::Private
{
    KoPatternSP patternResource;
};

PsdPattern::PsdPattern()
    : d(new Private())
{
    d->patternResource = 0;
}

PsdPattern::~PsdPattern()
{
    delete d;
}

void PsdPattern::setPattern(KoPatternSP pattern)
{
    d->patternResource = pattern;
}

KoPatternSP PsdPattern::pattern() const
{
    return d->patternResource;
}


bool psd_write_pattern(QIODevice *io)
{
    Q_UNUSED(io);
    return false;
}


bool psd_read_pattern(QIODevice *io)
{
    quint32 pattern_length;
    psd_pattern pattern;

    psdread(io, &pattern_length);
    pattern_length = (pattern_length + 3) & ~3;
    psdread(io, &pattern.version);
    if (pattern.version != 1) return false;
    psdread(io, (quint32*)&pattern.color_mode);
    psdread(io, &pattern.height);
    psdread(io, &pattern.width);
    psdread_unicodestring(io, pattern.name);
    psdread_pascalstring(io, pattern.uuid, 2);

    if (pattern.color_mode == Indexed) {
        pattern.color_table.reserve(256);
        quint8 r;
        quint8 g;
        quint8 b;
        for (int i = 0; i < 256; ++i) {
            psdread(io, &r);
            psdread(io, &g);
            psdread(io, &b);
            pattern.color_table.append(qRgb(r, g, b));
        }
    }

    // Now load the virtual memory array
    psdread(io, &pattern.version);
    if (pattern.version != 3) return false;
    quint32 vm_array_length;
    psdread(io, &vm_array_length);
    psdread(io, &pattern.top);
    psdread(io, &pattern.left);
    psdread(io, &pattern.bottom);
    psdread(io, &pattern.right);

    QImage img;

    if (pattern.color_mode == Indexed) {
        img = QImage(pattern.width, pattern.height, QImage::Format_Indexed8);
        img.setColorTable(pattern.color_table);
    }
    else {
        img = QImage(pattern.width, pattern.height, QImage::Format_ARGB32);
    }


    qint32 max_channels;
    psdread(io, &max_channels);

    QVector<QByteArray> channelData;

    for (int i = 0; i < max_channels; ++i) {
        quint32 written;
        qint32 len;
        quint32 pixel_depth1;
        quint32 top;
        quint32 left;
        quint32 bottom;
        quint32 right;
        quint16 pixel_depth2;
        quint8 compression_mode;

        psdread(io, &written);
        psdread(io, &len);
        len -= 4 + 4 * 4 + 2 + 1;
        if (len < 0) {
            continue;
        }

        if (written > 0) {
            // Note: channel_number is not read from the file, so always equals 0.
            // Other behavior may be implemented later.
            if (pattern.channel_number == 0) {
                psdread(io, &pixel_depth1);
            }
            else {
                quint32 d;
                psdread(io, &d);
            }
        }
        else {
            quint32 d;
            psdread(io, &d);
        }

        psdread(io, &top);
        psdread(io, &left);
        psdread(io, &bottom);
        psdread(io, &right);
        psdread(io, &pixel_depth2);
        psdread(io, &compression_mode);

        quint32 per_channel_length = 0;

        if (written > 0) {
            if (pattern.channel_number == 0) {
                qint32 pixels;
                qint32 length;
                pixels = length = pattern.width * pattern.height;
                switch(pixel_depth1) {
                case 1:
                    length = (pattern.width + 7) / 8 * pattern.height;
                    break;
                case 8:
                    break;
                case 16:
                    length *= 2;
                    pixels *= 2;
                    break;
                default:
                    dbgKrita << "Wrong depth for pattern";
                    return false;
                }

                per_channel_length = length;
                Q_UNUSED(per_channel_length); // wtf!?

                switch(pattern.color_mode) {
                case Bitmap:
                case Indexed:
                    break;
                case Grayscale:
                case DuoTone:
                    length *= 2;
                    break;
                case RGB:
                    length *= 4;
                    break;
                case CMYK:
                    length *= 5;
                    break;
                case Lab:
                    length *= 4;
                    break;
                case MultiChannel:
                    length *= 4;
                    break;
                default:
                    dbgKrita << "Impossible color mode" << pattern.color_mode;
                    return false;
                }

                QByteArray ba = io->read(len);
                channelData << Compression::uncompress(length, ba, (Compression::CompressionType)compression_mode);
            }
        }
    }

    return true;
}
