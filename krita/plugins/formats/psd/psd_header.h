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
#ifndef PSD_HEADER_H
#define PSD_HEADER_H

#include <QDebug>
#include <QtGlobal>

#include "psd.h"

class QIODevice;

class PSDHeader
{
public:

    PSDHeader();

    /**
     * Reads a psd header from the given device.
     *
     * @return false if:
     *   <li>reading failed
     *   <li>if the 8BPS signature is not found
     *   <li>if the version is not 1 or 2
     */
    bool read(QIODevice* device);

    /**
     * write the header data to the given device
     *
     * @return false if writing failed or if this is not a valid header
     */
    bool write(QIODevice* device);

    bool valid();

    QString signature; // 8PBS
    quint16 version;   // 1 or 2
    quint16 nChannels; // 1 - 56
    quint32 height;    // 1-30,000 or 1 - 300,000
    quint32 width;     // 1-30,000 or 1 - 300,000
    quint16 channelDepth; // 1, 8, 16. XXX: check whether 32 is used!
    PSDColorMode colormode;

    QString error;

};

QDebug operator<<(QDebug dbg, const PSDHeader& header);

#endif // PSD_HEADER_H
