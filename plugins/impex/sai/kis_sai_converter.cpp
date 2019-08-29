/*
 *  Copyright (c) 2019 Wolthera van Hövell tot Westerflier
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "sai.hpp"

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <KisDocument.h>

#include <kis_transaction.h>
#include <kis_paint_device.h>

#include <kis_paint_layer.h>
#include <kis_node.h>
#include <kis_group_layer.h>

#include "kis_sai_converter.h"


KisSaiConverter::KisSaiConverter(KisDocument *doc)
    : QObject(0),
      m_doc(doc)
{

}

KisImportExportErrorCode KisSaiConverter::buildImage(const QString &filename)
{
    sai::Document saiFile(QFile::encodeName(filename));
    if (!saiFile.IsOpen()) {
        dbgFile << "Could not open the file, either it does not exist, either it is not a Sai file :" << filename;
        return ImportExportCodes::FileFormatIncorrect;
    }
    std::tuple<std::uint32_t, std::uint32_t> size = saiFile.GetCanvasSize();
    m_image = new KisImage(m_doc->createUndoStore(),
                                int(std::get<0>(size)),
                                int(std::get<1>(size)),
                                KoColorSpaceRegistry::instance()->rgb8(),
                                "file");
    return ImportExportCodes::OK;
}

KisImageSP KisSaiConverter::image()
{
    return m_image;
}
