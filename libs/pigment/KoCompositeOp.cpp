/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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

#include <klocale.h>

#include "KoCompositeOp.h"
#include "KoColorSpace.h"

KoCompositeOp::KoCompositeOp()
{

}


KoCompositeOp::KoCompositeOp(KoColorSpace * cs, const QString& id,  const QString& description,  const bool userVisible)
    : m_colorSpace( cs )
    , m_id( id )
    , m_description( description )
    , m_userVisible( userVisible )
{
}

void KoCompositeOp::composite(quint8 *dstRowStart, qint32 dstRowStride,
                              const quint8 *srcRowStart, qint32 srcRowStride,
                              const quint8 *maskRowStart, qint32 maskRowStride,
                              qint32 rows, qint32 numColumns,
                              quint8 opacity) const
{

    // XXX: For filters the convention is to pass an emtpy channelflags
    // parameter so the code can short-circuit if everything needs to
    // be filtered: should we do the same here? (BSAR)
    if (m_defaultChannelFlags.isNull() || m_defaultChannelFlags.isEmpty()) {
        m_defaultChannelFlags.fill( true, m_colorSpace->channelCount() );
    }

    composite( dstRowStart, dstRowStride,
               srcRowStart, srcRowStride,
               maskRowStart, maskRowStride,
               rows, numColumns,
               opacity, m_defaultChannelFlags);
}

