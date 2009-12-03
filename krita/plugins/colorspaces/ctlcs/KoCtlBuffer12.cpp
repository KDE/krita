/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "KoCtlBuffer.h"

KoCtlBuffer::KoCtlBuffer(char* _buffer, int _size) : m_buffer(_buffer), m_size(_size)
{
}
KoCtlBuffer::~KoCtlBuffer() {}
char * KoCtlBuffer::rawData()
{
    return m_buffer;
}
const char * KoCtlBuffer::rawData() const
{
    return m_buffer;
}
int KoCtlBuffer::size() const
{
    return m_size;
}

