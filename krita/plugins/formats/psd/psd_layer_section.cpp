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
}

PSDLayerSection::~PSDLayerSection()
{
    qDeleteAll(layerInfo.layers);
}

bool PSDLayerSection::read(QIODevice* io)
{
    {
        layerSectionSize = 0;
        if (m_header.m_version == 1) {
            quint32 _layerSectionSize;
            if (!psdread(io, &_layerSectionSize) || layerSectionSize > (quint64)io->bytesAvailable()) {
                error = "Could not read layer section size";
                return false;
            }
            layerSectionSize = _layerSectionSize;
        }
        else if (m_header.m_version == 2) {
            if (!psdread(io, &layerSectionSize) || layerSectionSize > (quint64)io->bytesAvailable()) {
                error = "Could not read layer section size";
                return false;
            }
        }

    }
    { // Layer Info block

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

        // rounded to a multiple of 2
        if ((layerInfo.layerInfoSize & 0x01) != 0) {
            layerInfo.layerInfoSize++;
        }

        if (!psdread(io, &layerInfo.nLayers) || layerInfo.nLayers == 0) {
            error = "Could not read read number of layers or no layers in image.";
            return false;
        }

        if (layerInfo.nLayers < 0) {
            layerInfo.hasTransparency = true; // first alpha channel is the alpha channel of the projection.
            layerInfo.nLayers = -layerInfo.nLayers;
        }
        else {
            layerInfo.hasTransparency = false;
        }

        for (int i = 0; i < layerInfo.nLayers; ++i) {
            PSDLayerRecord *layerRecord = new PSDLayerRecord(m_header);
            if (!layerRecord->read(io)) {
                error = QString("Could not load layer %1").arg(i);
                return false;
            }
            layerInfo.layers << layerRecord;
        }
    }

    {
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
    if (layerInfo.nLayers <= 0) return false;
    if (layerInfo.nLayers != layerInfo.layers.size()) return false;
    foreach(PSDLayerRecord* layer, layerInfo.layers) {
        if (!layer->valid()) {
            return false;
        }
    }
    if (!error.isNull()) {
        return false;
    }
    return true;
}
