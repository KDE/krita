/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_REPEAT_ITERATORS_PIXEL_H_
#define _KIS_REPEAT_ITERATORS_PIXEL_H_

#include <QRect>

template<class T>
class KisRepeatHLineIteratorPixelBase;
template<class T>
class KisRepeatVLineIteratorPixelBase;

/**
 * This iterator is an iterator that will "artificially" extend the paint device with the
 * value of the border when trying to access values outside the range of data.
 */
template<class T>
class KisRepeatLineIteratorPixelBase
{
    friend class KisRepeatHLineIteratorPixelBase<T>;
    friend class KisRepeatVLineIteratorPixelBase<T>;
    /**
     * @param rc indicates the rectangle that truly contains data
     */
    inline KisRepeatLineIteratorPixelBase(KisDataManager *dm, KisDataManager *sel_dm, qint32 x, qint32 y, qint32 offsetx, qint32 offsety, const QRect& _rc);
    inline ~KisRepeatLineIteratorPixelBase();
public:
    inline qint32 x() const {
        return m_realX;
    }
    inline qint32 y() const {
        return m_realY;
    }
    inline const quint8 * oldRawData() const {
        return m_iterator->oldRawData();
    }
private:
    KisDataManager* m_dm;
    KisDataManager* m_sel_dm;
    qint32 m_realX, m_realY;
    qint32 m_offsetX, m_offsetY;
    QRect m_dataRect;
    T* m_iterator;
};

/**
 * This iterator is an iterator that will "artificially" extend the paint device with the
 * value of the border when trying to access values outside the range of data.
 */
template<class T>
class KisRepeatHLineIteratorPixelBase : public KisRepeatLineIteratorPixelBase<T>
{
public:
    /**
     * @param rc indicates the rectangle that trully contains data
     */
    inline KisRepeatHLineIteratorPixelBase(KisDataManager *dm, KisDataManager *sel_dm, qint32 x, qint32 y, qint32 w, qint32 offsetx, qint32 offsety, const QRect& _rc);
    inline ~KisRepeatHLineIteratorPixelBase();
    inline KisRepeatHLineIteratorPixelBase<T> & operator ++();
    inline bool isDone() const {
        return this->m_realX >= m_startX + m_width;
    }
    /**
     * Reach next row.
     */
    inline void nextRow();
private:
    void createIterator();
private:
    qint32 m_startX;
    qint32 m_startIteratorX;
    qint32 m_width;
};

/**
 * This iterator is an iterator that will "artificially" extend the paint device with the
 * value of the border when trying to access values outside the range of data.
 */
template<class T>
class KisRepeatVLineIteratorPixelBase : public KisRepeatLineIteratorPixelBase<T>
{
public:
    /**
     * @param rc indicates the rectangle that trully contains data
     */
    inline KisRepeatVLineIteratorPixelBase(KisDataManager *dm, KisDataManager *sel_dm, qint32 x, qint32 y, qint32 h, qint32 offsetx, qint32 offsety, const QRect& _rc);
    inline ~KisRepeatVLineIteratorPixelBase();
    inline KisRepeatVLineIteratorPixelBase<T> & operator ++();
    inline bool isDone() const {
        return this->m_realY >= m_startY + m_height;
    }
    /**
     * Reach next row.
     */
    inline void nextCol();
private:
    void createIterator();
private:
    qint32 m_startY;
    qint32 m_startIteratorY;
    qint32 m_height;
};

//------------------------ Implementations ------------------------//

//---------------- KisRepeatLineIteratorPixelBase -----------------//

template<class T>
KisRepeatLineIteratorPixelBase<T>::KisRepeatLineIteratorPixelBase(KisDataManager *dm, KisDataManager *sel_dm, qint32 x, qint32 y, qint32 offsetx, qint32 offsety, const QRect& _rc) :
        m_dm(dm), m_sel_dm(sel_dm),
        m_realX(x), m_realY(y),
        m_offsetX(offsetx), m_offsetY(offsety),
        m_dataRect(_rc), m_iterator(0)
{
}

template<class T>
KisRepeatLineIteratorPixelBase<T>::~KisRepeatLineIteratorPixelBase()
{
    delete m_iterator;
}

//---------------- KisRepeatHLineIteratorPixelBase ----------------//

template<class T>
KisRepeatHLineIteratorPixelBase<T>::KisRepeatHLineIteratorPixelBase(KisDataManager *dm, KisDataManager *sel_dm, qint32 x, qint32 y, qint32 w, qint32 offsetx, qint32 offsety, const QRect& _rc) : KisRepeatLineIteratorPixelBase<T>(dm, sel_dm, x, y, offsetx, offsety , _rc),
        m_startX(x), m_startIteratorX(x),
        m_width(w)
{
    // Compute the startx value of the iterator
    if (m_startIteratorX < _rc.left()) {
        m_startIteratorX = _rc.left();
    }
    createIterator();
}

template<class T>
KisRepeatHLineIteratorPixelBase<T>::~KisRepeatHLineIteratorPixelBase()
{
}

template<class T>
inline KisRepeatHLineIteratorPixelBase<T> & KisRepeatHLineIteratorPixelBase<T>::operator++()
{
    Q_ASSERT(this->m_iterator);
    if (this->m_realX >= this->m_dataRect.x() && this->m_realX < this->m_dataRect.x() + this->m_dataRect.width() - 1) {
        ++(*this->m_iterator);
    }
    ++this->m_realX;
    return *this;
}

template<class T>
inline void KisRepeatHLineIteratorPixelBase<T>::nextRow()
{
    if (this->m_realY >= this->m_dataRect.y() && this->m_realY < this->m_dataRect.y() + this->m_dataRect.height()) {
        this->m_iterator->nextRow();
    } else {
        createIterator();
    }
    ++this->m_realY;
}

template<class T>
void KisRepeatHLineIteratorPixelBase<T>::createIterator()
{
    // Cleanup
    delete this->m_iterator;
    qint32 startY = this->m_realY;
    if (startY < this->m_dataRect.y()) {
        startY = this->m_dataRect.top();
    }
    if (startY > (this->m_dataRect.y() + this->m_dataRect.height() - 1)) {
        startY = (this->m_dataRect.y() + this->m_dataRect.height() - 1);
    }
    this->m_iterator = new T(this->m_dm, this->m_sel_dm, this->m_startIteratorX, startY, this->m_dataRect.width(), this->m_offsetX, this->m_offsetY);
    this->m_realX = this->m_startX;
}

//---------------- KisRepeatVLineIteratorPixelBase ----------------//

template<class T>
KisRepeatVLineIteratorPixelBase<T>::KisRepeatVLineIteratorPixelBase(KisDataManager *dm, KisDataManager *sel_dm, qint32 x, qint32 y, qint32 h, qint32 offsetx, qint32 offsety, const QRect& _rc) : KisRepeatLineIteratorPixelBase<T>(dm, sel_dm, x, y, offsetx, offsety , _rc),
        m_startY(y), m_startIteratorY(y),
        m_height(h)
{
    // Compute the startx value of the iterator
    if (m_startIteratorY < _rc.top()) {
        m_startIteratorY = _rc.top();
    }
    createIterator();
}

template<class T>
KisRepeatVLineIteratorPixelBase<T>::~KisRepeatVLineIteratorPixelBase()
{
}

template<class T>
inline KisRepeatVLineIteratorPixelBase<T> & KisRepeatVLineIteratorPixelBase<T>::operator++()
{
    Q_ASSERT(this->m_iterator);
    if (this->m_realY >= this->m_dataRect.y() && this->m_realY < this->m_dataRect.y() + this->m_dataRect.height() - 1) {
        ++(*this->m_iterator);
    }
    ++this->m_realY;
    return *this;
}

template<class T>
inline void KisRepeatVLineIteratorPixelBase<T>::nextCol()
{
    if (this->m_realX >= this->m_dataRect.x() && this->m_realX < this->m_dataRect.x() + this->m_dataRect.width()) {
        this->m_iterator->nextCol();
    } else {
        createIterator();
    }
    ++this->m_realX;
}

template<class T>
void KisRepeatVLineIteratorPixelBase<T>::createIterator()
{
    // Cleanup
    delete this->m_iterator;
    qint32 startX = this->m_realX;
    if (startX < this->m_dataRect.x()) {
        startX = this->m_dataRect.x();
    }
    if (startX > (this->m_dataRect.x() + this->m_dataRect.width() - 1)) {
        startX = (this->m_dataRect.x() + this->m_dataRect.width() - 1);
    }
    this->m_iterator = new T(this->m_dm, this->m_sel_dm, startX, this->m_startIteratorY, this->m_dataRect.height(), this->m_offsetX, this->m_offsetY);
    this->m_realY = this->m_startY;
}


#endif
