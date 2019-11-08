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
#include <vpx/vpx_encoder.h>

class Encoder
{
    int m_width;
    int m_height;
    vpx_codec_ctx m_codec;
    vpx_image_t m_raw;
    vpx_codec_err_t m_res;
    vpx_codec_enc_cfg_t m_cfg;
    int m_frameCount = 0;
    int m_keyframeInterval = 4;
    FILE* m_file = nullptr;

public:
    Encoder()
    {
    }

    void init(const char* filename, int width, int height);

    void pushFrame(uint8_t* data[3], uint32_t size);

    void finish();
};

#endif
