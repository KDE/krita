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
#include <array>
#include <QSemaphore>
#include <QThread>
#include <atomic>

class Encoder : QThread
{
    Q_OBJECT
    class RingBufferItem
    {
    public:
        enum class Command{
            Payload,
            Finish
        };

        Command m_command;
        uint8_t *m_payload;
    };

    unsigned int m_width;
    unsigned int m_height;
    vpx_codec_ctx m_codec;
    vpx_image_t m_raw;
    vpx_codec_enc_cfg_t m_cfg;
    int m_frameCount = 0;
    int m_keyframeInterval = 4;
    static constexpr int m_ringBufferSize = 5;
    std::array<RingBufferItem, m_ringBufferSize> m_ringBuffer;
    int m_head = 0;
    int m_tail = 0;
    QSemaphore m_full;
    QSemaphore m_empty;
    QString m_filename;
    std::atomic<bool> *m_shouldFinish;

public:
    Encoder()
    {
    }

    void run() override;

    void init(const QString &filename, unsigned int width, unsigned int height);

    void pushFrame(uint8_t* data, unsigned int m_width, unsigned int m_height, uint32_t size);

    void finish();
};

#endif
