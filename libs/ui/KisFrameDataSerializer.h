/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KISFRAMEDATASERIALIZER_H
#define KISFRAMEDATASERIALIZER_H

#include "kritaui_export.h"
#include <QScopedPointer>
#include "opengl/kis_texture_tile_info_pool.h"

// TODO: extract DataBuffer into a separate file
#include "opengl/kis_texture_tile_update_info.h"

#include <vector>
#include <boost/optional.hpp>

class QString;


/**
 * KisFrameDataSerializer is the lowest level class for storing frame
 * data on disk. Its responsibilities are simple:
 *
 * 1) Accept low-level frame data object (KisFrameDataSerializer::Frame),
 *    which contains raw data in it (the data may be not a pixel data,
 *    but a preprocessed pixel differences)
 *
 * 2) Compress this data and save it on disk
 */

class KRITAUI_EXPORT KisFrameDataSerializer
{
public:
    struct FrameTile
    {
        FrameTile(KisTextureTileInfoPoolSP pool) : data(pool) {}

        FrameTile(FrameTile &&rhs) = default;
        FrameTile& operator=(FrameTile &&rhs) = default;

        FrameTile(const FrameTile &rhs) = delete;
        FrameTile& operator=(FrameTile &rhs) = delete;

        bool isValid() const {
            return data.data();
        }

        FrameTile clone() const{
            FrameTile tile(data.pool());
            tile.col = col;
            tile.row = row;
            tile.rect = rect;
            tile.data.allocate(data.pixelSize());

            const int bufferSize = data.pixelSize() * rect.width() * rect.height();
            memcpy(tile.data.data(), data.data(), bufferSize);

            return tile;
        }

        int col = -1;
        int row = -1;
        bool isCompressed = false;
        QRect rect;
        DataBuffer data;
    };

    struct Frame
    {
        Frame() = default;

        Frame(Frame&&rhs) = default;
        Frame& operator=(Frame &&rhs) = default;

        Frame(const Frame &rhs) = delete;
        Frame& operator=(Frame &rhs) = delete;

        Frame clone() const {
            Frame frame;
            frame.pixelSize = pixelSize;
            for (auto it = frameTiles.begin(); it != frameTiles.end(); ++it) {
                frame.frameTiles.push_back(it->clone());
            }
            return frame;
        }

        int pixelSize = 0;
        std::vector<FrameTile> frameTiles;

        bool isValid() const {
            return pixelSize > 0;
        }
    };

public:
    KisFrameDataSerializer();
    KisFrameDataSerializer(const QString &frameCachePath);
    ~KisFrameDataSerializer();

    int saveFrame(const Frame &frame);
    Frame loadFrame(int frameId, KisTextureTileInfoPoolSP pool);

    void moveFrame(int srcFrameId, int dstFrameId);

    bool hasFrame(int frameId) const;
    void forgetFrame(int frameId);

    static boost::optional<qreal> estimateFrameUniqueness(const Frame &lhs, const Frame &rhs, qreal portion);
    static bool subtractFrames(Frame &dst, const Frame &src);
    static void addFrames(Frame &dst, const Frame &src);

private:
    template<template <typename U> class OpPolicy>
    static bool processFrames(KisFrameDataSerializer::Frame &dst, const KisFrameDataSerializer::Frame &src);

private:
    Q_DISABLE_COPY(KisFrameDataSerializer)

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISFRAMEDATASERIALIZER_H
