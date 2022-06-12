/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_buffer_stream.h"

#include <kis_debug.h>

KisBufferStreamContigBase::KisBufferStreamContigBase(uint8_t *src,
                                                     uint16_t depth,
                                                     tsize_t lineSize)

    : KisBufferStreamBase(depth)
    , m_src(src)
    , m_srcIt(src)
    , m_posinc(8)
    , m_lineSize(lineSize)
{
    KIS_ASSERT(depth <= 32);
    restart();
}

void KisBufferStreamContigBase::restart()
{
    m_srcIt = m_src;
    m_lineOffset = 0;
    m_lineNumber = 0;
    m_posinc = 8;
}

void KisBufferStreamContigBase::moveToLine(tsize_t lineNumber)
{
    KIS_ASSERT(lineNumber >= 0);
    moveToPos(0, lineNumber);
}

void KisBufferStreamContigBase::moveToPos(tsize_t x, tsize_t y)
{
    KIS_ASSERT(x >= 0 && y >= 0);
    m_lineNumber = y;
    m_lineOffset = (x * m_depth) / 8;
    m_srcIt = m_src + y * m_lineSize + m_lineOffset;
    m_posinc = 8;
}

tsize_t KisBufferStreamContigBase::x() const
{
    return (m_lineOffset * 8) / m_depth;
}

tsize_t KisBufferStreamContigBase::y() const
{
    return m_lineNumber;
}

tsize_t KisBufferStreamContigBase::width() const
{
    return (m_lineSize * 8) / m_depth;
}

uint32_t KisBufferStreamContigBelow16::nextValue()
{
    uint16_t remain = m_depth;
    uint32_t value =  0 ;
    while (remain > 0) {
        uint16_t toread = remain;
        if (toread > m_posinc) toread = m_posinc;
        remain -= toread;
        m_posinc -= toread;
        value = (value << toread) | (((*m_srcIt) >> (m_posinc)) & ((1 << toread) - 1));
        if (m_posinc == 0) {
            m_srcIt++;
            m_lineOffset++; // we consumed a byte
            m_posinc = 8;
        }
    }
    if (m_lineOffset >= m_lineSize) {
        m_lineNumber += 1;
        m_lineOffset = 0;
    }
    return value;
}

uint32_t KisBufferStreamContigBelow32::nextValue()
{
    uint16_t remain = m_depth;
    uint32_t value = 0;
    while (remain > 0) {
        uint16_t toread = remain;
        if (toread > m_posinc) toread = m_posinc;
        remain -= toread;
        m_posinc -= toread;
        value = (value) | ((((*m_srcIt) >> (m_posinc)) & ((1 << toread) - 1U)) << (m_depth - 8U - remain));
        if (m_posinc == 0) {
            m_srcIt++;
            m_lineOffset++; // we consumed a byte
            m_posinc = 8U;
        }
    }
    if (m_lineOffset >= m_lineSize) {
        m_lineNumber += 1;
        m_lineOffset = 0;
    }
    return value;
}

uint32_t KisBufferStreamContigAbove32::nextValue()
{
    uint16_t remain = m_depth;
    uint32_t value = 0;
    while (remain > 0) {
        uint16_t toread = remain;
        if (toread > m_posinc)
            toread = m_posinc;
        remain -= toread;
        m_posinc = m_posinc - toread;
        if (remain < 32U) {
            value |= (((*m_srcIt >> m_posinc) & ((1U << toread) - 1U)) << (24U - remain));
        }
        if (m_posinc == 0) {
            m_srcIt++;
            m_lineOffset++; // we consumed a byte
            m_posinc = 8U;
        }
    }
    if (m_lineOffset >= m_lineSize) {
        m_lineNumber += 1;
        m_lineOffset = 0;
    }
    return value;
}

KisBufferStreamSeparate::KisBufferStreamSeparate(uint8_t **srcs, uint16_t nb_samples, uint16_t depth, tsize_t *lineSize)
    : KisBufferStreamBase(depth)
    , m_nb_samples(nb_samples)
{
    if (depth < 16) {
        for (uint16_t i = 0; i < m_nb_samples; i++) {
            streams.push_back(
                QSharedPointer<KisBufferStreamContigBelow16>::create(
                    srcs[i],
                    depth,
                    lineSize[i]));
        }
    } else if (depth < 32) {
        for (uint16_t i = 0; i < m_nb_samples; i++) {
            streams.push_back(
                QSharedPointer<KisBufferStreamContigBelow32>::create(
                    srcs[i],
                    depth,
                    lineSize[i]));
        }
    } else {
        for (uint16_t i = 0; i < m_nb_samples; i++) {
            streams.push_back(
                QSharedPointer<KisBufferStreamContigAbove32>::create(
                    srcs[i],
                    depth,
                    lineSize[i]));
        }
    }
    restart();
}

uint32_t KisBufferStreamSeparate::nextValue()
{
    const uint32_t value = streams[m_current_sample]->nextValue();
    if ((++m_current_sample) >= m_nb_samples)
        m_current_sample = 0;
    return value;
}

void KisBufferStreamSeparate::restart()
{
    m_current_sample = 0;
    for (const auto &stream : streams) {
        stream->restart();
    }
}

void KisBufferStreamSeparate::moveToLine(tsize_t lineNumber)
{
    KIS_ASSERT(lineNumber >= 0);
    moveToPos(0, lineNumber);
}

void KisBufferStreamSeparate::moveToPos(tsize_t x, tsize_t y)
{
    for (const auto &stream : streams) {
        stream->moveToPos(x, y);
    }
}

tsize_t KisBufferStreamSeparate::x() const
{
    return streams[m_current_sample]->x();
}

tsize_t KisBufferStreamSeparate::y() const
{
    return streams[m_current_sample]->y();
}

tsize_t KisBufferStreamSeparate::width() const
{
    return streams[m_current_sample]->width();
}

KisBufferStreamInterleaveUpsample::KisBufferStreamInterleaveUpsample(
    uint8_t **srcs,
    uint16_t nb_samples,
    uint16_t depth,
    tsize_t *lineSize,
    uint16_t hsubsample,
    uint16_t vsubsample)
    : KisBufferStreamSeparate(srcs, nb_samples, depth, lineSize)
    , m_hsubsample(hsubsample)
    , m_vsubsample(vsubsample)
{
}

uint32_t KisBufferStreamInterleaveUpsample::nextValue()
{
    uint32_t value = streams[m_currentPlane]->nextValue();
    if (m_currentPlane == 0) {
        m_current_sample++;
        if (m_current_sample % m_hsubsample == 0) {
            if (m_current_sample >= m_hsubsample * m_vsubsample) {
                // Fix up the position of the luminance plane
                // If it's already 0, the cursor has already looped (correctly)
                // to the next line
                if (streams[m_currentPlane]->x() != 0) {
                    streams[m_currentPlane]->moveToPos(
                        streams[m_currentPlane]->x(),
                        streams[m_currentPlane]->y() - m_vsubsample + 1);
                }
                // Move to Cb/Cr
                m_currentPlane += 1;
                m_current_sample = 0;
            } else {
                // Go to next line
                // If the position is already 0, we need to correct the row
                // AND column
                if (streams[m_currentPlane]->x() != 0) {
                    streams[m_currentPlane]->moveToPos(
                        streams[m_currentPlane]->x() - m_hsubsample,
                        streams[m_currentPlane]->y() + 1);
                } else {
                    streams[m_currentPlane]->moveToPos(
                        streams[m_currentPlane]->width() - m_hsubsample,
                        streams[m_currentPlane]->y());
                }
            }
        }
    } else if (m_currentPlane < m_nb_samples - 1) {
        m_currentPlane += 1;
    } else {
        m_currentPlane = 0;
    }
    return value;
}

void KisBufferStreamInterleaveUpsample::moveToPos(tsize_t x, tsize_t y)
{
    // Needs to subsample
    for (uint16_t i = 0; i < m_nb_samples; i++) {
        const tsize_t realX = i == 0 ? x : x / m_hsubsample;
        const tsize_t realY = i == 0 ? y : y / m_vsubsample;
        streams.at(i)->moveToPos(realX, realY);
    }
}

tsize_t KisBufferStreamInterleaveUpsample::x() const
{
    return streams[0]->x();
}

tsize_t KisBufferStreamInterleaveUpsample::y() const
{
    return streams[0]->y();
}
