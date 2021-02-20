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

#include <tiffio.h>

class KisBufferStreamBase
{
public:
    KisBufferStreamBase(uint16 depth) : m_depth(depth) {}
    virtual uint32 nextValue() = 0;
    virtual void restart() = 0;
    virtual void moveToLine(uint32 lineNumber) = 0;
    virtual ~KisBufferStreamBase() {}
protected:
    uint16 m_depth;
};

class KisBufferStreamContigBase : public KisBufferStreamBase
{
public:
    KisBufferStreamContigBase(uint8* src, uint16 depth, uint32 lineSize);
    void restart() override;
    void moveToLine(uint32 lineNumber) override;
    ~KisBufferStreamContigBase() override {}
protected:
    uint8* m_src;
    uint8* m_srcIt;
    uint8 m_posinc;
    uint32 m_lineSize;
};

class KisBufferStreamContigBelow16 : public KisBufferStreamContigBase
{
public:
    KisBufferStreamContigBelow16(uint8* src, uint16 depth, uint32 lineSize) : KisBufferStreamContigBase(src, depth, lineSize) { }
public:
    ~KisBufferStreamContigBelow16() override {}
    uint32 nextValue() override;
};

class KisBufferStreamContigBelow32 : public KisBufferStreamContigBase
{
public:
    KisBufferStreamContigBelow32(uint8* src, uint16 depth, uint32 lineSize) : KisBufferStreamContigBase(src, depth, lineSize) { }
public:
    ~KisBufferStreamContigBelow32() override {}
    uint32 nextValue() override;
};

class KisBufferStreamContigAbove32 : public KisBufferStreamContigBase
{
public:
    KisBufferStreamContigAbove32(uint8* src, uint16 depth, uint32 lineSize) : KisBufferStreamContigBase(src, depth, lineSize) { }
public:
    ~KisBufferStreamContigAbove32() override {}
    uint32 nextValue() override;
};


class KisBufferStreamSeperate : public KisBufferStreamBase
{
public:
    KisBufferStreamSeperate(uint8** srcs, uint8 nb_samples , uint16 depth, uint32* lineSize);
    ~KisBufferStreamSeperate() override;
    uint32 nextValue() override;
    void restart() override;
    void moveToLine(uint32 lineNumber) override;
private:
    KisBufferStreamContigBase** streams;
    uint8 m_current_sample, m_nb_samples;
};

#endif
