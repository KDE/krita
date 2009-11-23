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
#ifndef PSD_LAYER_SECTION_H
#define PSD_LAYER_SECTION_H

#include <QString>

class QIODevice;

#include "psd_header.h"
#include "psd_layer_record.h"

class PSDLayerSection
{
public:
    PSDLayerSection(const PSDHeader& header);
    bool read(QIODevice* io);
    bool write(QIODevice* io);
    bool valid();

    QString error;

    qint16 nLayers;
    QVector<PSDLayerRecord> layers;

private:

    const PSDHeader m_header;

};

#endif // PSD_LAYER_SECTION_H
