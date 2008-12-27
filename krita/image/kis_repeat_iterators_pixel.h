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

/**
 * This iterator is an iterator that will "artificially" extend the paint device with the
 * value of the border when trying to access values outside the range of data.
 */
template<class T>
class KisRepeatHLineIteratorPixelBase {
    public:
        /**
         * @param rc indicates the rectangle that trully contains data
         */
        inline KisRepeatHLineIteratorPixelBase( KisDataManager *dm, KisDataManager *sel_dm, qint32 x, qint32 y, qint32 w, qint32 offsetx, qint32 offsety, const QRect& _rc );
        inline ~KisRepeatHLineIteratorPixelBase();
        inline KisRepeatHLineIteratorPixelBase<T> & operator ++();
        inline qint32 x() const {
            return m_realX;
        }
        inline qint32 y() const {
            return m_realY;
        }
        inline const quint8 * oldRawData() const {
            return m_iterator->oldRawData();
        }
        inline bool isDone() const {
            return m_realX >= m_startX + m_width;
        }
        /**
         * Reach next row.
         */
        inline void nextRow();
    private:
        void createIterator();
    private:
        KisDataManager* m_dm;
        KisDataManager* m_sel_dm;
        qint32 m_startX;
        qint32 m_startIteratorX;
        qint32 m_realX, m_realY;
        qint32 m_width;
        qint32 m_offsetX, m_offsetY;
        QRect m_dataRect;
        T* m_iterator;
};

template<class T>
KisRepeatHLineIteratorPixelBase<T>::KisRepeatHLineIteratorPixelBase( KisDataManager *dm, KisDataManager *sel_dm, qint32 x, qint32 y, qint32 w, qint32 offsetx, qint32 offsety, const QRect& _rc ) :
        m_dm(dm), m_sel_dm(sel_dm),
        m_startX(x), m_startIteratorX(x),
        m_realX(x), m_realY(y),
        m_width(w),
        m_offsetX(offsetx), m_offsetY(offsety),
        m_dataRect(_rc), m_iterator(0)
{
    // Compute the startx value of the iterator
    if( m_startIteratorX < _rc.left() )
    {
        m_startIteratorX = _rc.left();
    }
    createIterator();
}

template<class T>
KisRepeatHLineIteratorPixelBase<T>::~KisRepeatHLineIteratorPixelBase()
{
    delete m_iterator;
}

template<class T>
inline KisRepeatHLineIteratorPixelBase<T> & KisRepeatHLineIteratorPixelBase<T>::operator++()
{
    Q_ASSERT(m_iterator);
    if( m_realX >= m_dataRect.x() && m_realX < m_dataRect.x() + m_dataRect.width() )
    {
        ++(*m_iterator);
    }
    ++m_realX;
    return *this;
}

template<class T>
inline void KisRepeatHLineIteratorPixelBase<T>::nextRow()
{
    if( m_realY >= m_dataRect.y() && m_realY < m_dataRect.y() + m_dataRect.height() )
    {
        m_iterator->nextRow();
    } else {
        createIterator();
    }
    ++m_realY;
    return *this;
}

template<class T>
void KisRepeatHLineIteratorPixelBase<T>::createIterator()
{
    // Cleanup
    delete m_iterator;
    qint32 startY = m_realY;
    if( startY < m_dataRect.y() ) {
        startY = m_dataRect.top();
    }
    if( startY > (m_dataRect.y() + m_dataRect.height() - 1 ) ) {
        startY = (m_dataRect.y() + m_dataRect.height() - 1 );
    }
    m_iterator = new T( m_dm, m_sel_dm, m_startIteratorX, startY, m_dataRect.width(), m_offsetX, m_offsetY );
    m_realX = m_startX;
}

#endif
