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
#include "psd_layer_section.h"

#include <QIODevice>

#include <kis_debug.h>

#include "psd_header.h"
#include "psd_utils.h"

PSDLayerSection::PSDLayerSection(const PSDHeader& header)
    : error(QString::null)
    , m_header(header)
{
    layerInfo.hasTransparency = false;
    layerInfo.layerInfoSize = 0;
    layerInfo.nLayers = 0;
}

PSDLayerSection::~PSDLayerSection()
{
    qDeleteAll(layerInfo.layers);
}

bool PSDLayerSection::read(QIODevice* io)
{
    dbgFile << "reading layer section. Pos:" << io->pos() <<  "bytes left:" << io->bytesAvailable();
    
    layerSectionSize = 0;
    if (m_header.m_version == 1) {
        quint32 _layerSectionSize = 0;
        if (!psdread(io, &_layerSectionSize) || _layerSectionSize > (quint64)io->bytesAvailable()) {
            error = QString("Could not read layer section size. Got %1. Bytes left %2")
                    .arg(_layerSectionSize).arg(io->bytesAvailable());
            return false;
        }
        layerSectionSize = _layerSectionSize;
    }
    else if (m_header.m_version == 2) {
        if (!psdread(io, &layerSectionSize) || layerSectionSize > (quint64)io->bytesAvailable()) {
            error = QString("Could not read layer section size. Got %1. Bytes left %2")
                    .arg(layerSectionSize).arg(io->bytesAvailable());
            return false;
        }
    }
    
    dbgFile << "layer section size" << layerSectionSize;
    
    dbgFile << "reading layer info block. Bytes left" << io->bytesAvailable() << "position" << io->pos();
    
    layerInfo.layerInfoSize = 0;
    layerInfo.nLayers = 0;
    if (m_header.m_version == 1) {
        quint32 layerInfoSize;
        if (!psdread(io, &layerInfoSize) || layerInfoSize > (quint64)io->bytesAvailable()) {
            error = "Could not read layer section size";
            return false;
        }
        layerInfo.layerInfoSize = layerInfoSize;
    }
    
    else if (m_header.m_version == 2) {
        if (!psdread(io, &layerInfo.layerInfoSize) || layerInfo.layerInfoSize > (quint64)io->bytesAvailable()) {
            error = "Could not read layer section size";
            return false;
        }
    }
    
    dbgFile << "Layer info block size" << layerInfo.layerInfoSize;
    
    if (layerInfo.layerInfoSize > 0 ) {
        
        // rounded to a multiple of 2
        if ((layerInfo.layerInfoSize & 0x01) != 0) {
            layerInfo.layerInfoSize++;
        }
        
        dbgFile << "Layer info block size after rounding" << layerInfo.layerInfoSize;
        
        if (!psdread(io, &layerInfo.nLayers) || layerInfo.nLayers == 0) {
            error = QString("Could not read read number of layers or no layers in image. %1").arg(layerInfo.nLayers);
            return false;
        }
        
        if (layerInfo.nLayers < 0) {
            layerInfo.hasTransparency = true; // first alpha channel is the alpha channel of the projection.
            layerInfo.nLayers = -layerInfo.nLayers;
        }
        else {
            layerInfo.hasTransparency = false;
        }

        dbgFile << "Number of layers" << layerInfo.nLayers << "transparency" << layerInfo.hasTransparency;

        for (int i = 0; i < layerInfo.nLayers; ++i) {
            PSDLayerRecord *layerRecord = new PSDLayerRecord(m_header, layerInfo.hasTransparency);
            if (!layerRecord->read(io)) {
                error = QString("Could not load layer %1: %2").arg(i).arg(layerRecord->error);
                return false;
            }
            dbgFile << "Read layer" << i << layerRecord->layerName;
            layerInfo.layers << layerRecord;
        }
    }

    // get the positions for the channels belonging to each layer
    for (int i = 0; i < layerInfo.nLayers; ++i) {

    }

    quint32 globalMaskBlockLength;
    if (!psdread(io, &globalMaskBlockLength) || globalMaskBlockLength > (quint64)io->bytesAvailable()) {
        error = "Could not read global mask info block";
        return false;
    }

    if (globalMaskBlockLength > 0) {

        if (!psdread(io, &maskInfo.overlayColorSpace)) {
            error = "Could not read global mask info overlay colorspace";
            return false;
        }

        for (int i = 0; i < 4; ++i) {
            if (!psdread(io, &maskInfo.colorComponents[i])) {
                error = QString("Could not read mask info visualizaion color component %1").arg(i);
                return false;
            }
        }

        if (!psdread(io, &maskInfo.opacity)) {
            error = "Could not read global mask info visualisation opacity";
            return false;
        }

        if (!psdread(io, &maskInfo.kind)) {
            error = "Could not read global mask info visualisation type";
            return false;
        }
    }

    return valid();
}

bool PSDLayerSection::write(QIODevice* io)
{
    Q_UNUSED(io);
    Q_ASSERT(valid());
    if (!valid()) {
        error = "Cannot write an invalid Layer Section object";
        return false;
    }
    qFatal("TODO: implement writing the layer section");
    return false;
}

bool PSDLayerSection::valid()
{
    if (layerInfo.layerInfoSize > 0) {
        if (layerInfo.nLayers <= 0) return false;
        if (layerInfo.nLayers != layerInfo.layers.size()) return false;
        foreach(PSDLayerRecord* layer, layerInfo.layers) {
            if (!layer->valid()) {
                return false;
            }
        }
    }
    if (!error.isNull()) {
        return false;
    }
    return true;
}
