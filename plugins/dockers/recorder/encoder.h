/*
 *  Copyright (c) 2019 Shi Yan <billconan@gmail.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef ENCODER_H
#define ENCODER_H

#include <fstream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h>
}

class Encoder
{
    int m_width;
    int m_height;
    std::string m_filename;
    int m_frameCount;
    AVFrame* m_frame = nullptr;
    AVCodecContext* m_codecContext = nullptr;
    SwsContext* m_swsContext = nullptr;
    AVFormatContext* m_formatContext = nullptr;
    AVOutputFormat* m_outputFormat = nullptr;
    AVStream* m_stream = nullptr;
    int m_fps = 4;
    int m_bitrate = 2000;

public:
    Encoder()
        : m_filename()
    {
    }

    void init(const std::string &filename, int width, int height);
    void pushFrame(uint8_t *frame);
    void finish();
};

#endif
