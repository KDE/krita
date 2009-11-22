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
#include "psd_resource_section.h"

#include "psd_utils.h"
#include <QIODevice>
#include <QBuffer>

PSDResourceSection::PSDResourceSection()
{
}

bool PSDResourceSection::read(QIODevice* io)
{
    quint32 resourceBlockLength;
    if (!psdread(io, resourceBlockLength)) {
        error = "Could not read resource block length";
        return false;
    }

    QByteArray ba = io->read(resourceBlockLength);
    if ((quint32)ba.size() != resourceBlockLength) {
        error = "Could not read all resources";
        return false;
    }

    QBuffer buf;
    buf.setBuffer(ba);

    return valid();
}

bool PSDResourceSection::write(QIODevice* io)
{
    Q_ASSERT(valid());
    if (!valid()) return false;
    qFatal("TODO: implement writing the resource section");
}

bool PSDResourceSection::valid()
{
    return true;
}
