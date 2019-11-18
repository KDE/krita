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

#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

class Encoder
{
    GstPipeline* m_pipeline;
    GstAppSrc* m_src;
    GstElement* m_filter1;
    GstElement* m_encoder;
    GstElement* m_videoconvert;
    GstElement* m_queue;
    GstElement* m_webmmux;
    GstElement* m_sink;
    int m_width;
    int m_height;
    GstClockTime m_timestamp;
    std::string m_filename;
    int m_frameCount;

public:
    Encoder()
        : m_pipeline(nullptr)
        , m_src(nullptr)
        , m_filter1(nullptr)
        , m_encoder(nullptr)
        , m_videoconvert(nullptr)
        , m_queue(nullptr)
        , m_webmmux(nullptr)
        , m_sink(nullptr)
        , m_filename()
    {
    }

    void init(const std::string &filename, int width, int height);
    void pushFrame(gpointer data, gsize size);
    void finish();
};

#endif
