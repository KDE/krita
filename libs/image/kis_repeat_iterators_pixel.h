/*
 *  SPDX-FileCopyrightText: 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_REPEAT_ITERATORS_PIXEL_H_
#define _KIS_REPEAT_ITERATORS_PIXEL_H_

#include <QRect>
#include "kis_shared.h"
#include "tiles3/kis_hline_iterator.h"
#include "tiles3/kis_vline_iterator.h"

template<class T>
class KisRepeatHLineIteratorPixelBase;
template<class T>
class KisRepeatVLineIteratorPixelBase;

/**
 * This iterator is an iterator that will "artificially" extend the paint device with the
 * value of the border when trying to access values outside the range of data.
 */
template<class T>
class KisRepeatLineIteratorPixelBase : public KisShared
{
    Q_DISABLE_COPY(KisRepeatLineIteratorPixelBase)
public:

    friend class KisRepeatHLineIteratorPixelBase<T>;
    friend class KisRepeatVLineIteratorPixelBase<T>;
    /**
     * @param dm data manager
     * @param x x of top left corner
     * @param y y of top left corner
     * @param offsetx x offset
     * @param offsety y offset
     * @param _rc indicates the rectangle that truly contains data
     * @param completeListener completion listener
     */
    inline KisRepeatLineIteratorPixelBase(KisDataManager *dm, qint32 x, qint32 y, qint32 offsetx, qint32 offsety, const QRect& _rc, KisIteratorCompleteListener *completeListener);
    virtual inline ~KisRepeatLineIteratorPixelBase();
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
    qint32 m_realX, m_realY;
    qint32 m_offsetX, m_offsetY;
    QRect m_dataRect;
    T* m_iterator;
    KisIteratorCompleteListener *m_completeListener;
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
     * @param dm data manager
     * @param x x of top left corner
     * @param y y of top left corner
     * @param w width
     * @param offsetx x offset
     * @param offsety y offset
     * @param _rc indicates the rectangle that truly contains data
     * @param completeListener completion listener
     */
    inline KisRepeatHLineIteratorPixelBase(KisDataManager *dm, qint32 x, qint32 y, qint32 w, qint32 offsetx, qint32 offsety, const QRect& _rc, KisIteratorCompleteListener *completeListener);
    inline ~KisRepeatHLineIteratorPixelBase() override;
    inline bool nextPixel();
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
     * @param dm data manager
     * @param x x of top left corner
     * @param y y of top left corner
     * @param h height
     * @param offsetx x offset
     * @param offsety y offset
     * @param _rc indicates the rectangle that truly contains data
     * @param completeListener completion listener
     */
    inline KisRepeatVLineIteratorPixelBase(KisDataManager *dm, qint32 x, qint32 y, qint32 h, qint32 offsetx, qint32 offsety, const QRect& _rc, KisIteratorCompleteListener *completeListener);
    inline ~KisRepeatVLineIteratorPixelBase() override;
    inline KisRepeatVLineIteratorPixelBase<T> & operator ++();
    inline bool nextPixel();
    /**
     * Reach next row.
     */
    inline void nextColumn();
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
KisRepeatLineIteratorPixelBase<T>::KisRepeatLineIteratorPixelBase(KisDataManager *dm, qint32 x, qint32 y, qint32 offsetx, qint32 offsety, const QRect& _rc, KisIteratorCompleteListener *completeListener) :
    m_dm(dm),
    m_realX(x), m_realY(y),
    m_offsetX(offsetx), m_offsetY(offsety),
    m_dataRect(_rc),
    m_iterator(0),
    m_completeListener(completeListener)
{
}

template<class T>
KisRepeatLineIteratorPixelBase<T>::~KisRepeatLineIteratorPixelBase()
{
    delete m_iterator;
}

//---------------- KisRepeatHLineIteratorPixelBase ----------------//

template<class T>
KisRepeatHLineIteratorPixelBase<T>::KisRepeatHLineIteratorPixelBase(KisDataManager *dm, qint32 x, qint32 y, qint32 w, qint32 offsetx, qint32 offsety, const QRect& _rc, KisIteratorCompleteListener *completeListener)
    : KisRepeatLineIteratorPixelBase<T>(dm, x, y, offsetx, offsety , _rc, completeListener),
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
inline bool KisRepeatHLineIteratorPixelBase<T>::nextPixel()
{
    Q_ASSERT(this->m_iterator);
    if (this->m_realX >= this->m_dataRect.x() && this->m_realX < this->m_dataRect.x() + this->m_dataRect.width() - 1) {
        this->m_iterator->nextPixel();
    }
    ++this->m_realX;

    return (this->m_realX < m_startX + m_width);
}

template<class T>
inline void KisRepeatHLineIteratorPixelBase<T>::nextRow()
{
    if (this->m_realY >= this->m_dataRect.y() && this->m_realY < this->m_dataRect.y() + this->m_dataRect.height() - 1) {
        this->m_iterator->nextRow();
    } else {
        createIterator();
    }
    this->m_realX = this->m_startX;
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

    int width = this->m_dataRect.x() + this->m_dataRect.width() - this->m_startIteratorX;
    this->m_iterator = new T(this->m_dm, this->m_startIteratorX, startY, width, this->m_offsetX, this->m_offsetY, false, this->m_completeListener);
    this->m_realX = this->m_startX;
}

//---------------- KisRepeatVLineIteratorPixelBase ----------------//

template<class T>
KisRepeatVLineIteratorPixelBase<T>::KisRepeatVLineIteratorPixelBase(KisDataManager *dm, qint32 x, qint32 y, qint32 h, qint32 offsetx, qint32 offsety, const QRect& _rc, KisIteratorCompleteListener *completeListener)
    : KisRepeatLineIteratorPixelBase<T>(dm, x, y, offsetx, offsety , _rc, completeListener),
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
inline bool KisRepeatVLineIteratorPixelBase<T>::nextPixel()
{
    Q_ASSERT(this->m_iterator);
    if (this->m_realY >= this->m_dataRect.y() && this->m_realY < this->m_dataRect.y() + this->m_dataRect.height() - 1) {
        this->m_iterator->nextPixel();
    }
    ++this->m_realY;
    return (this->m_realY < m_startY + m_height);

}

template<class T>
inline void KisRepeatVLineIteratorPixelBase<T>::nextColumn()
{
    if (this->m_realX >= this->m_dataRect.x() && this->m_realX < this->m_dataRect.x() + this->m_dataRect.width() - 1) {
        this->m_iterator->nextColumn();
    } else {
        createIterator();
    }
    this->m_realY = this->m_startY;
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

    int height = this->m_dataRect.y() + this->m_dataRect.height() - this->m_startIteratorY;
    this->m_iterator = new T(this->m_dm, startX, this->m_startIteratorY, height, this->m_offsetX, this->m_offsetY, false, this->m_completeListener);
    this->m_realY = this->m_startY;
}


#endif
