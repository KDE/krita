/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _KO_CTL_ACCUMULATOR_H_
#define _KO_CTL_ACCUMULATOR_H_

#include "KoColorSpaceMaths.h"

class KoCtlAccumulator
{
public:
    virtual ~KoCtlAccumulator();
    virtual void mix(const quint8* _pixel, double _weight) = 0;
    virtual void reset() = 0;
    virtual void affect(quint8* _pixel, double _alpha) = 0;
    virtual void affect(quint8* _pixel, qint32 factor, qint32 offset) = 0;
};

template<typename _type_>
class KoCtlAccumulatorImpl : public KoCtlAccumulator
{
public:
    KoCtlAccumulatorImpl(int _pos) : m_pos(_pos) {}
    virtual ~KoCtlAccumulatorImpl() {}
    inline const _type_* ptr(const quint8* _pixel) {
        return reinterpret_cast<const _type_*>(_pixel + m_pos);
    }
    inline _type_* ptr(quint8* _pixel) {
        return reinterpret_cast<_type_*>(_pixel + m_pos);
    }
    virtual void mix(const quint8* _pixel, double _weight) {
        m_value += *ptr(_pixel) * _weight;
    }
    virtual void reset() {
        m_value = 0;
    }
    virtual void affect(quint8* _pixel, double _alpha) {
        typename KoColorSpaceMathsTraits< _type_ >::compositetype v = m_value * _alpha;

        if (v > KoColorSpaceMathsTraits<_type_>::max) {
            v = KoColorSpaceMathsTraits<_type_>::max;
        }
        if (v < KoColorSpaceMathsTraits<_type_>::min) {
            v = KoColorSpaceMathsTraits<_type_>::min;
        }
        *ptr(_pixel) = v;
    }
    virtual void affect(quint8* _pixel, qint32 factor, qint32 offset) {
        typename KoColorSpaceMathsTraits< _type_ >::compositetype v = m_value / factor + offset;

        if (v > KoColorSpaceMathsTraits<_type_>::max) {
            v = KoColorSpaceMathsTraits<_type_>::max;
        }
        if (v < KoColorSpaceMathsTraits<_type_>::min) {
            v = KoColorSpaceMathsTraits<_type_>::min;
        }
        *ptr(_pixel) = v;
    }
private:
    int m_pos;
    typename KoColorSpaceMathsTraits< _type_ >::compositetype m_value;
};

#endif
