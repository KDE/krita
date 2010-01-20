/*
 *  Copyright (c) 2009 Boudewijn Rempt <boud@valdyas.org>
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
#include "psd_colormode_block.h"

#include "psd.h"
#include "psd_utils.h"

PSDColorModeBlock::PSDColorModeBlock(PSDColorMode colormode)
    : m_blocksize(0)
    , m_colormode(colormode)
{
}

bool PSDColorModeBlock::read(QIODevice* io)
{
    // get length
    psdread(io, &m_blocksize);

    if (m_blocksize == 0) {
        if (m_colormode == Indexed || m_colormode == DuoTone) {
            error = "Blocksize of 0 and Indexed or DuoTone colormode";
            return false;
        }
        else {
            return true;
        }
    }
    if (m_colormode == Indexed && m_blocksize != 768) {
        error = QString("Indexed mode, but block size is %1.").arg(m_blocksize);
        return false;
    }

    m_data = io->read(m_blocksize);
    if ((quint32)m_data.size() != m_blocksize) return false;

    if (m_colormode == Indexed) {
        qFatal("TODO: Compute the colormap");
        return false;
    }
    return valid();
}



bool PSDColorModeBlock::write(QIODevice* io)
{
    Q_UNUSED(io);
    Q_ASSERT(valid());
    if (!valid()) {
        error = "Cannot write an invalid Color Mode Block";
        return false;
    }
    qFatal("TODO: implement writing the colormode block");
    return false;
}

bool PSDColorModeBlock::valid()
{
    if (m_blocksize == 0 && (m_colormode == Indexed || m_colormode == DuoTone)) {
        error = "Blocksize of 0 and Indexed or DuoTone colormode";
        return false;
    }
    if (m_colormode == Indexed && m_blocksize != 768) {
        error = QString("Indexed mode, but block size is %1.").arg(m_blocksize);
        return false;
    }
    if (m_colormode == DuoTone && m_blocksize == 0) {
        error == QString("DuoTone mode, but data block is empty");
        return false;
    }
    if ((quint32)m_data.size() != m_blocksize) {
        error = QString("Data size is %1, but block size is %2").arg(m_data.size()).arg(m_blocksize);
        return false;
    }
    return true;
}
