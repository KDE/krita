/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_buffer_stream.h"

KisBufferStreamContigBase::KisBufferStreamContigBase(uint8* src, uint16 depth, uint32 lineSize) : KisBufferStreamBase(depth), m_src(src), m_lineSize(lineSize)
{
    restart();
}

void KisBufferStreamContigBase::restart()
{
    m_srcIt = m_src;
    m_posinc = 8;
}

void KisBufferStreamContigBase::moveToLine(uint32 lineNumber)
{
    m_srcIt = m_src + lineNumber * m_lineSize;
    m_posinc = 8;
}

uint32 KisBufferStreamContigBelow16::nextValue()
{
    uint8 remain;
    uint32 value;
    remain = (uint8) m_depth;
    value = 0;
    while (remain > 0) {
        uint8 toread;
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

uint32 KisBufferStreamContigBelow32::nextValue()
{
    uint8 remain;
    uint32 value;
    remain = (uint8) m_depth;
    value = 0;
    while (remain > 0) {
        uint8 toread;
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

uint32 KisBufferStreamContigAbove32::nextValue()
{
    uint8 remain;
    uint32 value;
    remain = (uint8) m_depth;
    value = 0;
    while (remain > 0) {
        uint8 toread;
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

KisBufferStreamSeperate::KisBufferStreamSeperate(uint8** srcs, uint8 nb_samples , uint16 depth, uint32* lineSize) : KisBufferStreamBase(depth), m_nb_samples(nb_samples)
{
    streams = new KisBufferStreamContigBase*[nb_samples];
    if (depth < 16) {
        for (uint8 i = 0; i < m_nb_samples; i++) {
            streams[i] = new KisBufferStreamContigBelow16(srcs[i], depth, lineSize[i]);
        }
    } else if (depth < 32) {
        for (uint8 i = 0; i < m_nb_samples; i++) {
            streams[i] = new KisBufferStreamContigBelow32(srcs[i], depth, lineSize[i]);
        }
    } else {
        for (uint8 i = 0; i < m_nb_samples; i++) {
            streams[i] = new KisBufferStreamContigAbove32(srcs[i], depth, lineSize[i]);
        }
    }
    restart();
}

KisBufferStreamSeperate::~KisBufferStreamSeperate()
{
    for (uint8 i = 0; i < m_nb_samples; i++) {
        delete streams[i];
    }
    delete[] streams;
}

uint32 KisBufferStreamSeperate::nextValue()
{
    uint32 value = streams[ m_current_sample ]->nextValue();
    if ((++m_current_sample) >= m_nb_samples)
        m_current_sample = 0;
    return value;
}

void KisBufferStreamSeperate::restart()
{
    m_current_sample = 0;
    for (uint8 i = 0; i < m_nb_samples; i++) {
        streams[i]->restart();
    }
}

void KisBufferStreamSeperate::moveToLine(uint32 lineNumber)
{
    for (uint8 i = 0; i < m_nb_samples; i++) {
        streams[i]->moveToLine(lineNumber);
    }
}
