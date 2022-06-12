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
    m_posinc = 8;
}

void KisBufferStreamContigBase::moveToLine(tsize_t lineNumber)
{
    m_srcIt = m_src + lineNumber * m_lineSize;
    m_posinc = 8;
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
            m_posinc = 8;
        }
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
            m_posinc = 8U;
        }
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
            m_posinc = 8U;
        }
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
    for (const auto &stream : streams) {
        stream->moveToLine(lineNumber);
    }
}
