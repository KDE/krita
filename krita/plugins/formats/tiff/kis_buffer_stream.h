/*
 *  Copyright (c) 2005-2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KIS_BUFFER_STREAM_H_
#define _KIS_BUFFER_STREAM_H_

#ifdef _MSC_VER // this removes KDEWIN extensions to stdio.h: required by tiffio.h
#define KDEWIN_STDIO_H
#include <../include/stdio.h>
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
    virtual void restart();
    virtual void moveToLine(uint32 lineNumber);
    virtual ~KisBufferStreamContigBase() {}
protected:
    uint8* m_src;
    uint8* m_srcit;
    uint8 m_posinc;
    uint32 m_lineSize;
};

class KisBufferStreamContigBelow16 : public KisBufferStreamContigBase
{
public:
    KisBufferStreamContigBelow16(uint8* src, uint16 depth, uint32 lineSize) : KisBufferStreamContigBase(src, depth, lineSize) { }
public:
    virtual ~KisBufferStreamContigBelow16() {}
    virtual uint32 nextValue();
};

class KisBufferStreamContigBelow32 : public KisBufferStreamContigBase
{
public:
    KisBufferStreamContigBelow32(uint8* src, uint16 depth, uint32 lineSize) : KisBufferStreamContigBase(src, depth, lineSize) { }
public:
    virtual ~KisBufferStreamContigBelow32() {}
    virtual uint32 nextValue();
};

class KisBufferStreamContigAbove32 : public KisBufferStreamContigBase
{
public:
    KisBufferStreamContigAbove32(uint8* src, uint16 depth, uint32 lineSize) : KisBufferStreamContigBase(src, depth, lineSize) { }
public:
    virtual ~KisBufferStreamContigAbove32() {}
    virtual uint32 nextValue();
};


class KisBufferStreamSeperate : public KisBufferStreamBase
{
public:
    KisBufferStreamSeperate(uint8** srcs, uint8 nb_samples , uint16 depth, uint32* lineSize);
    virtual ~KisBufferStreamSeperate();
    virtual uint32 nextValue();
    virtual void restart();
    virtual void moveToLine(uint32 lineNumber);
private:
    KisBufferStreamContigBase** streams;
    uint8 m_current_sample, m_nb_samples;
};

#endif
