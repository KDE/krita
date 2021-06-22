/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_BUFFER_STREAM_H_
#define _KIS_BUFFER_STREAM_H_

#ifdef _MSC_VER
#define KDEWIN_STDIO_H  // Remove KDEWIN extensions to stdio.h
#include <stdio.h>
#endif

#include <cstdint>
#include <tiffio.h>

class KisBufferStreamBase
{
public:
    KisBufferStreamBase(uint16_t depth) : m_depth(depth) {}
    virtual uint32_t nextValue() = 0;
    virtual void restart() = 0;
    virtual void moveToLine(tsize_t lineNumber) = 0;
    virtual ~KisBufferStreamBase() {}
protected:
    uint16_t m_depth;
};

class KisBufferStreamContigBase : public KisBufferStreamBase
{
public:
    KisBufferStreamContigBase(uint8_t *src, uint16_t depth, tsize_t lineSize);
    void restart() override;
    void moveToLine(tsize_t lineNumber) override;
    ~KisBufferStreamContigBase() override {}
protected:
    uint8_t* m_src;
    uint8_t* m_srcIt;
    uint8_t m_posinc;
    tsize_t m_lineSize;
};

class KisBufferStreamContigBelow16 : public KisBufferStreamContigBase
{
public:
    KisBufferStreamContigBelow16(uint8_t *src, uint16_t depth, tsize_t lineSize)
        : KisBufferStreamContigBase(src, depth, lineSize)
    {
    }

public:
    ~KisBufferStreamContigBelow16() override {}
    uint32_t nextValue() override;
};

class KisBufferStreamContigBelow32 : public KisBufferStreamContigBase
{
public:
    KisBufferStreamContigBelow32(uint8_t *src, uint16_t depth, tsize_t lineSize)
        : KisBufferStreamContigBase(src, depth, lineSize)
    {
    }

public:
    ~KisBufferStreamContigBelow32() override {}
    uint32_t nextValue() override;
};

class KisBufferStreamContigAbove32 : public KisBufferStreamContigBase
{
public:
    KisBufferStreamContigAbove32(uint8_t *src, uint16_t depth, tsize_t lineSize)
        : KisBufferStreamContigBase(src, depth, lineSize)
    {
    }

public:
    ~KisBufferStreamContigAbove32() override {}
    uint32_t nextValue() override;
};


class KisBufferStreamSeparate : public KisBufferStreamBase
{
public:
    KisBufferStreamSeparate(uint8_t **srcs, uint16_t nb_samples, uint16_t depth, tsize_t *lineSize);
    ~KisBufferStreamSeparate() override;
    uint32_t nextValue() override;
    void restart() override;
    void moveToLine(tsize_t lineNumber) override;

private:
    KisBufferStreamContigBase** streams;
    uint16_t m_current_sample, m_nb_samples;
};

#endif
