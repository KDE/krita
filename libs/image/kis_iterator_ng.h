/* This file is part of the KDE project
 *  SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ITERATOR_NG_H_
#define _KIS_ITERATOR_NG_H_

#include "kis_base_accessor.h"

class KRITAIMAGE_EXPORT KisBaseConstIteratorNG : public KisBaseConstAccessor
{
    Q_DISABLE_COPY(KisBaseConstIteratorNG)
public:
    KisBaseConstIteratorNG() {}
    ~KisBaseConstIteratorNG() override;
    /**
     * Move to the next pixel
     * @return false if there is no more pixel in the line
     */
    virtual bool nextPixel() = 0;
    /**
     * Move to the next pixels
     */
    virtual bool nextPixels(qint32 n) = 0;
    /**
     * @return return number of consequential numbers of pixels, useful for optimization
     */
    virtual qint32 nConseqPixels() const = 0;
};

//class KRITAIMAGE_EXPORT KisBaseIteratorNG : public virtual KisBaseConstIteratorNG, public virtual KisBaseAccessor
//{
//    Q_DISABLE_COPY(KisBaseIteratorNG)
//public:
//    KisBaseIteratorNG() {}
//    virtual ~KisBaseIteratorNG();
//};

/**
 * Iterates over the line of a paint device.
 */
class KRITAIMAGE_EXPORT KisHLineConstIteratorNG : public virtual KisBaseConstIteratorNG
{
    Q_DISABLE_COPY(KisHLineConstIteratorNG)
public:
    KisHLineConstIteratorNG() {}
    ~KisHLineConstIteratorNG() override;
    /**
     * Move to the next row
     */
    virtual void nextRow() = 0;

    virtual void resetPixelPos() = 0;
    virtual void resetRowPos() = 0;
};

/**
 * Also support writing.
 */
class KRITAIMAGE_EXPORT KisHLineIteratorNG : public KisHLineConstIteratorNG, public KisBaseAccessor
{
    Q_DISABLE_COPY(KisHLineIteratorNG)
public:
    KisHLineIteratorNG() {}
    ~KisHLineIteratorNG() override;
};

/**
 * Iterates over the column of a paint device.
 */
class KRITAIMAGE_EXPORT KisVLineConstIteratorNG : public virtual KisBaseConstIteratorNG
{
    Q_DISABLE_COPY(KisVLineConstIteratorNG)
public:
    KisVLineConstIteratorNG() {}
    ~KisVLineConstIteratorNG() override;
    /**
     * Move to the next row
     */
    virtual void nextColumn() = 0;

    virtual void resetPixelPos() = 0;
    virtual void resetColumnPos() = 0;
};

/**
 * Also support writing.
 */
class KRITAIMAGE_EXPORT KisVLineIteratorNG : public KisVLineConstIteratorNG, public KisBaseAccessor
{
    Q_DISABLE_COPY(KisVLineIteratorNG)
public:
    KisVLineIteratorNG() {}
    ~KisVLineIteratorNG() override;
};

#include "kis_sequential_iterator.h"

#endif
