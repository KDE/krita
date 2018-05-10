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


        int col = -1;
        int row = -1;
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

        int frameId = -1;
        int pixelSize = 0;
        std::vector<FrameTile> frameTiles;

        bool isValid() const {
            return frameId >= 0;
        }
    };

public:
    KisFrameDataSerializer(KisTextureTileInfoPoolSP pool);
    KisFrameDataSerializer(KisTextureTileInfoPoolSP pool, const QString &frameCachePath);
    ~KisFrameDataSerializer();

    void saveFrame(const Frame &frame);
    Frame loadFrame(int frameId);

    bool hasFrame(int frameId) const;
    void forgetFrame(int frameId);

    KisTextureTileInfoPoolSP tileInfoPool() const;

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
