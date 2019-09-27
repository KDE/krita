/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "jp2_converter.h"

#include <QFileInfo>
#include <QApplication>

#include <QMessageBox>

#include <QFileInfo>

#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceTraits.h>
#include <KoColorSpaceConstants.h>
#include <KisImportExportManager.h>
#include <KoColorSpace.h>
#include <KoColorModelStandardIds.h>

#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_paint_device.h>
#include <kis_transaction.h>
#include "kis_iterator_ng.h"

JP2Converter::JP2Converter(KisDocument *doc)
{
    m_doc = doc;
    m_stop = false;
}

JP2Converter::~JP2Converter()
{
}


KisImportExportErrorCode JP2Converter::buildImage(const QString &filename)
{
    // Determine the colorspace and channel depth

    // Get the appropriate KoColorSpace object from KoColorSpaceRegistry
    const KoColorSpace* cs = 0;

    // Get the dimensions
    int width, height = 0;

    // Create the image
    m_image = new KisImage(m_doc->createUndoStore(), width, height, cs, "built image");

    // Create a layer
    KisPaintLayerSP layer = new KisPaintLayer(m_image, m_image -> nextLayerName(), quint8_MAX);

    // Fill the layer with pixels
    // ...

    // Set the layer on the image
    m_image->addNode(layer);

    // And report the status
    return ImportExportCodes::OK;
}


KisImageWSP JP2Converter::image()
{
    return m_image;
}


KisImportExportErrorCode JP2Converter::buildFile(const QString &filename, KisPaintLayerSP layer, const JP2ConvertOptions& options)
{
    return ImportExportCodes::Failure;
}


void JP2Converter::cancel()
{
    m_stop = true;
}



