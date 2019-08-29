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

#include "kis_sai_import.h"

#include <QCheckBox>
#include <QSlider>
#include <QApplication>
#include <QFileInfo>
#include <QImageReader>

#include <kpluginfactory.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>

#include <kis_transaction.h>
#include <kis_paint_device.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_node.h>
#include <kis_group_layer.h>
#include <sai.hpp>


K_PLUGIN_FACTORY_WITH_JSON(KisQImageIOImportFactory, "krita_sai_import.json", registerPlugin<KisQImageIOImport>();)

KisQImageIOImport::KisQImageIOImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisQImageIOImport::~KisQImageIOImport()
{
}

KisImportExportErrorCode KisQImageIOImport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP /*configuration*/)
{
    sai::Document saiFile(QFile::encodeName(filename()));
    if (!saiFile.IsOpen()) {
        dbgFile << "Could not open the file, either it does not exist, either it is not a Sai file :" << filename();
        return ImportExportCodes::FileFormatIncorrect;
    }
    std::tuple<std::uint32_t, std::uint32_t> size = saiFile.GetCanvasSize();
    KisImageSP image = new KisImage(document->createUndoStore(),
                                    int(std::get<0>(size)),
                                    int(std::get<1>(size)),
                                    KoColorSpaceRegistry::instance()->rgb8(),
                                    "file");

    document->setCurrentImage(image);
    return ImportExportCodes::OK;
}

#include "kis_sai_import.moc"

