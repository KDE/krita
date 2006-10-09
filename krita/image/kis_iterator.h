/* This file is part of the KDE project
 *  Copyright (c) 2004 Casper Boemann <cbr@boemann.dkt>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#ifndef KIS_ITERATOR_H_
#define KIS_ITERATOR_H_

#include <qglobal.h>
#include <kis_shared_ptr.h>
#include <krita_export.h>

class KisTiledRectIterator;
typedef KisSharedPtr<KisTiledRectIterator> KisTiledRectIteratorSP;

class KisTiledVLineIterator;
typedef KisSharedPtr<KisTiledVLineIterator> KisTiledVLineIteratorSP;

class KisTiledHLineIterator;
typedef KisSharedPtr<KisTiledHLineIterator> KisTiledHLineIteratorSP;

class KisDataManager;

/**
 * The KisRectIterator iterators over a rectangular area in the most efficient order. That is,
 * there is no guarantee that the iterator will work scanline by scanline.
 */
class KRITAIMAGE_EXPORT KisRectConstIterator
{
    friend class KisRectIterator;
protected:
    KisRectConstIterator ( KisDataManager *dm, qint32  x, qint32  y, qint32  w, qint32  h, bool writable);

public:
    KisRectConstIterator ( KisDataManager *dm, qint32  x, qint32  y, qint32  w, qint32  h);

public:
    virtual ~KisRectConstIterator();
    KisRectConstIterator(const KisRectConstIterator& rhs);
    KisRectConstIterator& operator=(const KisRectConstIterator& rhs);


public:
    /// returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
    const quint8 * rawData() const;

    /// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
    const quint8 * oldRawData() const;

    /// Returns the number of consecutive pixels that we point at
    /// This is useful for optimizing
    qint32 nConseqPixels() const;

    /// Advances a number of pixels until it reaches the end of the rect
    KisRectConstIterator & operator+=(int n);

    /// Advances one pixel going to the beginning of the next line when it reaches the end of a line
    KisRectConstIterator & operator++();

    /// returns true when iterators has reached the end
    bool isDone()  const;

     // current x position
     qint32 x() const;

     // current y position
     qint32 y() const;

private:

    KisTiledRectIteratorSP m_iter;
};

class KRITAIMAGE_EXPORT KisRectIterator : public KisRectConstIterator {
    public:
        inline KisRectIterator ( KisDataManager *dm, qint32  x, qint32  y, qint32  w, qint32  h) : KisRectConstIterator(dm, x, y, w, h, true) { }
        /// returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
        quint8 * rawData() const;
        /// Advances a number of pixels until it reaches the end of the rect
        KisRectIterator & operator+=(int n) { KisRectConstIterator::operator+=(n); return *this; }

        /// Advances one pixel going to the beginning of the next line when it reaches the end of a line
        KisRectIterator & operator++() { KisRectConstIterator::operator++(); return *this; }

};

class KRITAIMAGE_EXPORT KisHLineConstIterator
{
    friend class KisHLineIterator;
protected:
    KisHLineConstIterator ( KisDataManager *dm, qint32  x, qint32 y, qint32 w, bool writable);

public:

    KisHLineConstIterator ( KisDataManager *dm, qint32  x, qint32 y, qint32 w);

public:

    virtual ~KisHLineConstIterator();
    KisHLineConstIterator(const KisHLineConstIterator& rhs);
    KisHLineConstIterator& operator=(const KisHLineConstIterator& rhs);

public:
    /// Returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
    const quint8 *rawData() const;

    /// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
    const quint8 *oldRawData() const;

    /// Advances one pixel until it reaches the end of the line
    KisHLineConstIterator & operator++();

    /// Returns the number of consecutive horizontal pixels that we point at
    /// This is useful for optimizing
    qint32 nConseqHPixels() const;

    /// Advances a number of pixels until it reaches the end of the line
    KisHLineConstIterator & operator+=(int n);

    /// Goes back one pixel until it reaches the beginning of the line
    KisHLineConstIterator & operator--();

    /// returns true when iterators has reached the end
    bool isDone()  const;

    /// current x position
    qint32 x() const;

    /// current y position
    qint32 y() const;

    /// increment to the next row and rewind to the beginning
    void nextRow();


private:

    KisTiledHLineIteratorSP m_iter;
};

class KRITAIMAGE_EXPORT KisHLineIterator : public KisHLineConstIterator {
    public:
        inline KisHLineIterator ( KisDataManager *dm, qint32  x, qint32 y, qint32 w) : KisHLineConstIterator(dm, x, y, w, true) { }
    public:
        /// Returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
        quint8 *rawData() const;
        
        /// Advances one pixel until it reaches the end of the line
        KisHLineIterator & operator++() { KisHLineConstIterator::operator++(); return *this; }

        /// Advances a number of pixels until it reaches the end of the line
        KisHLineConstIterator & operator+=(int n) { KisHLineConstIterator::operator+=(n); return *this; }

        /// Goes back one pixel until it reaches the beginning of the line
        KisHLineConstIterator & operator--() { KisHLineConstIterator::operator--(); return *this; }

};

class KRITAIMAGE_EXPORT KisVLineConstIterator
{
    friend class KisVLineIterator;
protected:
    KisVLineConstIterator ( KisDataManager *dm, qint32  x, qint32 y, qint32  h, bool writable);

public:
    KisVLineConstIterator ( KisDataManager *dm, qint32  x, qint32 y, qint32  h);
public:
    ~KisVLineConstIterator();
    KisVLineConstIterator(const KisVLineConstIterator& rhs);
    KisVLineConstIterator& operator=(const KisVLineConstIterator& rhs);

public:
    /// returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
    const quint8 *rawData() const;

    /// Returns a pointer to the pixel data as it was at the moment of the last memento creation.
    const quint8 * oldRawData() const;

    /// Advances one pixel until it reaches the end of the line
    KisVLineConstIterator & operator++();

    /// returns true when iterators has reached the end
    bool isDone() const;

    /// current x position
    qint32 x() const;

    /// current y position
    qint32 y() const;

    /// increment to the next column and rewind to the beginning
    void nextCol();

private:

    KisTiledVLineIteratorSP m_iter;

};

class KRITAIMAGE_EXPORT KisVLineIterator : public KisVLineConstIterator {
    public:
        inline KisVLineIterator ( KisDataManager *dm, qint32  x, qint32 y, qint32 h) : KisVLineConstIterator(dm, x, y, h, true) { }
        /// returns a pointer to the pixel data. Do NOT interpret the data - leave that to a colorstrategy
        quint8 *rawData() const;
        /// Advances one pixel until it reaches the end of the line
        KisVLineIterator & operator++() { KisVLineConstIterator::operator++(); return *this; }

};

#endif
