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
#include "psd_resource_block.h"

#include <QIODevice>

#include <kis_debug.h>

#include "psd_utils.h"

PSDResourceBlock::PSDResourceBlock()
    : m_identifier(PSDResourceSection::UNKNOWN)
{
}

bool PSDResourceBlock::read(QIODevice* io)
{
    if (io->atEnd()) {
        error = "Could not read resource block: no bytes left.";
        return false;
    }

    QByteArray b;
    b = io->read(4);
    if(b.size() != 4 || QString(b) != "8BIM") {
        error = QString("Could not read resource block signature. Got %1.")
                .arg(QString(b));
        return false;
    }

    if (!psdread(io, &m_identifier)) {
        error = "Could not read resource block identifier";
        return false;
    }

    //dbgFile << "resource block identifier" << m_identifier;

    if (!psdread_pascalstring(io, m_name)) {
        error = "Could not read name of resource block";
        return false;
    }

    //dbgFile << "resource block name" << m_name;

    if (!psdread(io, &m_dataSize)) {
        error = QString("Could not read datasize for resource block with name %1 of type %2").arg(m_name).arg(m_identifier);
        return false;
    }

    if ((m_dataSize & 0x01) != 0) {
        m_dataSize++;
    }

    //dbgFile << "resource block size" << m_dataSize;

    m_data = io->read(m_dataSize);
    if (!m_data.size() == m_dataSize) {
        error = QString("Could not read data for resource block with name %1 of type %2").arg(m_name).arg(m_identifier);
        return false;
    }

    return valid();
}

bool PSDResourceBlock::write(QIODevice* io)
{
    Q_UNUSED(io);
    Q_ASSERT(valid());
    if (!valid()) {
        error = QString("Cannot write an invalid Resource Block");
        return false;
    }
    qFatal("TODO: implement writing the resource block");
    return false;
}

bool PSDResourceBlock::valid()
{
    if (m_identifier == PSDResourceSection::UNKNOWN) {
        error = QString("Unknown ID: %1").arg(m_identifier);
        return false;
    }
    return true;
}

