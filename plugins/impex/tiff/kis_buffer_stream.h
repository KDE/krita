/*
 *  SPDX-FileCopyrightText: 2005-2006 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 L. E. Segovia <amy@amyspark.me>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_BUFFER_STREAM_H_
#define _KIS_BUFFER_STREAM_H_

#include <cstdint>

#include <QSharedPointer>
#include <QVector>

#include <tiffio.h>

class KisBufferStreamBase
{
public:
    KisBufferStreamBase(uint16_t depth) : m_depth(depth) {}
    virtual ~KisBufferStreamBase() = default;
    virtual uint32_t nextValue() = 0;
    virtual void restart() = 0;
    virtual void moveToLine(tsize_t lineNumber) = 0;
    virtual void moveToPos(tsize_t x, tsize_t y) = 0;
    virtual tsize_t x() const = 0;
    virtual tsize_t y() const = 0;
    virtual tsize_t width() const = 0;

protected:
    uint16_t m_depth;
};

class KisBufferStreamContigBase : public KisBufferStreamBase
{
public:
    KisBufferStreamContigBase(uint8_t *src, uint16_t depth, tsize_t lineSize);

    ~KisBufferStreamContigBase() override = default;

    void restart() override;

    void moveToLine(tsize_t lineNumber) override;

    void moveToPos(tsize_t x, tsize_t y) override;

    tsize_t x() const override;

    tsize_t y() const override;

    tsize_t width() const override;

protected:
    uint8_t *const m_src;
    uint8_t *m_srcIt;
    uint16_t m_posinc = 0;
    const tsize_t m_lineSize;
    tsize_t m_lineNumber = 0;
    tsize_t m_lineOffset = 0;
};

class KisBufferStreamContigBelow16 : public KisBufferStreamContigBase
{
public:
    KisBufferStreamContigBelow16(uint8_t *src, uint16_t depth, tsize_t lineSize)
        : KisBufferStreamContigBase(src, depth, lineSize)
    {
    }
    ~KisBufferStreamContigBelow16() override = default;
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
    ~KisBufferStreamContigBelow32() override = default;
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
    ~KisBufferStreamContigAbove32() override = default;
    uint32_t nextValue() override;
};

class KisBufferStreamSeparate : public KisBufferStreamBase
{
public:
    KisBufferStreamSeparate(uint8_t **srcs,
                            uint16_t nb_samples,
                            uint16_t depth,
                            tsize_t *lineSize);
    ~KisBufferStreamSeparate() override = default;

    uint32_t nextValue() override;

    void restart() override;

    void moveToLine(tsize_t lineNumber) override;

    void moveToPos(tsize_t x, tsize_t y) override;

    tsize_t x() const override;

    tsize_t y() const override;

    tsize_t width() const override;

protected:
    QVector<QSharedPointer<KisBufferStreamBase>> streams;
    uint16_t m_current_sample = 0;
    uint16_t m_nb_samples;
};

class KisBufferStreamInterleaveUpsample : public KisBufferStreamSeparate
{
public:
    KisBufferStreamInterleaveUpsample(uint8_t **srcs,
                                      uint16_t nb_samples,
                                      uint16_t depth,
                                      tsize_t *lineSize,
                                      uint16_t hsubsample,
                                      uint16_t vsubsample);

    uint32_t nextValue() override;

    void moveToPos(tsize_t x, tsize_t y) override;

    tsize_t x() const override;

    tsize_t y() const override;

    tsize_t width() const override
    {
        return streams[0]->width();
    }

    void restart() override
    {
        KisBufferStreamSeparate::restart();
        m_currentPlane = 0;
    }

protected:
    uint16_t m_hsubsample, m_vsubsample;
    uint16_t m_currentPlane = 0;
};

#endif
