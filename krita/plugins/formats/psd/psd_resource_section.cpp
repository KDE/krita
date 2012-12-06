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

#include <QDebug>
#include <QIODevice>
#include <QBuffer>

#include <kis_debug.h>

#include "psd_utils.h"
#include "psd_resource_block.h"

PSDResourceSection::PSDResourceSection()
{
}

PSDResourceSection::~PSDResourceSection()
{
    qDeleteAll(resources);
}

bool PSDResourceSection::read(QIODevice* io)
{
    quint32 resourceBlockLength = 0;
    if (!psdread(io, &resourceBlockLength)) {
        error = "Could not read resource block length";
        return false;
    }

    dbgFile << "Resource block length" << resourceBlockLength << "starts at" << io->pos();

    QByteArray ba = io->read(resourceBlockLength);
    if ((quint32)ba.size() != resourceBlockLength) {
        error = "Could not read all resources";
        return false;
    }

    QBuffer buf;
    buf.setBuffer(&ba);
    buf.open(QBuffer::ReadOnly);

    while (!buf.atEnd()) {
        PSDResourceBlock* block = new PSDResourceBlock();
        if (!block->read(&buf)) {
            error = "Error reading block: " + block->error;
            return false;
        }
        dbgFile << "resource block created. Type:" << block->identifier << "size"
                << block->dataSize
                << "," << buf.bytesAvailable() << "bytes to go";
        resources[(PSDResourceID)block->identifier] = block;
    }
    return valid();
}

bool PSDResourceSection::write(QIODevice* io)
{
    Q_UNUSED(io);

    if (!valid()) {
        error = "Resource Section is Invalid";
        return false;
    }
    // First write all the sections
    QByteArray ba;
    QBuffer buf;
    buf.setBuffer(&ba);
    buf.open(QBuffer::WriteOnly);

    foreach(PSDResourceBlock* block, resources) {
        if (!block->write(&buf)) {
            error = block->error;
            return false;
        }
    }

    buf.close();

    // Then get the size
    quint32 resourceBlockLength = ba.size();
    dbgFile << "resource section has size" << resourceBlockLength;
    psdwrite(io, resourceBlockLength);

    // and write the whole buffer;
    return (io->write(ba.constData(), ba.size()) == resourceBlockLength);

}

bool PSDResourceSection::valid()
{
    return true;
}
