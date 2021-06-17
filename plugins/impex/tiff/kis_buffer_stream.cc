/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_buffer_stream.h"

KisBufferStreamContigBase::KisBufferStreamContigBase(uint8_t* src, uint16_t depth, uint32_t lineSize) : KisBufferStreamBase(depth), m_src(src), m_lineSize(lineSize)
{
    restart();
}

void KisBufferStreamContigBase::restart()
{
    m_srcIt = m_src;
    m_posinc = 8;
}

void KisBufferStreamContigBase::moveToLine(uint32_t lineNumber)
{
    m_srcIt = m_src + lineNumber * m_lineSize;
    m_posinc = 8;
}

uint32_t KisBufferStreamContigBelow16::nextValue()
{
    uint8_t remain;
    uint32_t value;
    remain = (uint8_t) m_depth;
    value = 0;
    while (remain > 0) {
        uint8_t toread;
        toread = remain;
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
    uint8_t remain;
    uint32_t value;
    remain = (uint8_t) m_depth;
    value = 0;
    while (remain > 0) {
        uint8_t toread;
        toread = remain;
        if (toread > m_posinc) toread = m_posinc;
        remain -= toread;
        m_posinc -= toread;
        value = (value) | ((((*m_srcIt) >> (m_posinc)) & ((1 << toread) - 1)) << (m_depth - 8 - remain));
        if (m_posinc == 0) {
            m_srcIt++;
            m_posinc = 8;
        }
    }
    return value;
}

uint32_t KisBufferStreamContigAbove32::nextValue()
{
    uint8_t remain;
    uint32_t value;
    remain = (uint8_t) m_depth;
    value = 0;
    while (remain > 0) {
        uint8_t toread;
        toread = remain;
        if (toread > m_posinc) toread = m_posinc;
        remain -= toread;
        m_posinc -= toread;
        if (remain < 32) {
            value = (value) | ((((*m_srcIt) >> (m_posinc)) & ((1 << toread) - 1)) << (24 - remain));
        }
        if (m_posinc == 0) {
            m_srcIt++;
            m_posinc = 8;
        }
    }
    return value;
}

KisBufferStreamSeparate::KisBufferStreamSeparate(uint8_t** srcs, uint8_t nb_samples , uint16_t depth, uint32_t* lineSize) : KisBufferStreamBase(depth), m_nb_samples(nb_samples)
{
    streams = new KisBufferStreamContigBase*[nb_samples];
    if (depth < 16) {
        for (uint8_t i = 0; i < m_nb_samples; i++) {
            streams[i] = new KisBufferStreamContigBelow16(srcs[i], depth, lineSize[i]);
        }
    } else if (depth < 32) {
        for (uint8_t i = 0; i < m_nb_samples; i++) {
            streams[i] = new KisBufferStreamContigBelow32(srcs[i], depth, lineSize[i]);
        }
    } else {
        for (uint8_t i = 0; i < m_nb_samples; i++) {
            streams[i] = new KisBufferStreamContigAbove32(srcs[i], depth, lineSize[i]);
        }
    }
    restart();
}

KisBufferStreamSeparate::~KisBufferStreamSeparate()
{
    for (uint8_t i = 0; i < m_nb_samples; i++) {
        delete streams[i];
    }
    delete[] streams;
}

uint32_t KisBufferStreamSeparate::nextValue()
{
    uint32_t value = streams[ m_current_sample ]->nextValue();
    if ((++m_current_sample) >= m_nb_samples)
        m_current_sample = 0;
    return value;
}

void KisBufferStreamSeparate::restart()
{
    m_current_sample = 0;
    for (uint8_t i = 0; i < m_nb_samples; i++) {
        streams[i]->restart();
    }
}

void KisBufferStreamSeparate::moveToLine(uint32_t lineNumber)
{
    for (uint8_t i = 0; i < m_nb_samples; i++) {
        streams[i]->moveToLine(lineNumber);
    }
}
