/*
 *  Copyright (c) 2019 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KISCPPQUIRKS_H
#define KISCPPQUIRKS_H

namespace std {

#if __cplusplus < 201402L

template <typename Cont>
inline auto rbegin(Cont &cont) -> decltype(declval<Cont>().rbegin()) {
    return cont.rbegin();
}

template <typename Cont>
inline auto rend(Cont &cont) -> decltype(declval<Cont>().rend()) {
    return cont.rend();
}

template <class BidirectionalIterator>
inline reverse_iterator<BidirectionalIterator> make_reverse_iterator(BidirectionalIterator x)
{
    return reverse_iterator<BidirectionalIterator>(x);
}

#endif

}

#endif // KISCPPQUIRKS_H
