/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include <magickexport.h>
#include <kgenericfactory.h>
#include <KoDocument.h>
#include <KoFilterChain.h>
#include <KoColorSpace.h>

#include <kis_doc2.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_image.h>
#include <kis_annotation.h>
#include <kis_types.h>
#include <kis_image_magick_converter.h>

typedef KGenericFactory<MagickExport, KoFilter> MagickExportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritagmagickexport, MagickExportFactory("kofficefilters"))

MagickExport::MagickExport(QObject* parent, const QStringList&) : KoFilter(parent)
{
}

MagickExport::~MagickExport()
{
}

KoFilter::ConversionStatus MagickExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile << "magick export! From:" << from << ", To:" << to << "";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    // XXX: Add dialog about flattening layers here

    KisDoc2 *output = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!output)
        return KoFilter::CreationError;

    if (filename.isEmpty()) return KoFilter::FileNotFound;

    KUrl url;
    url.setPath(filename);

    KisImageWSP img = output->image();

    KisImageMagickConverter ib(output, output->undoAdapter());

    KisPaintDeviceSP pd = new KisPaintDevice(*img->projection());
    KisPaintLayerSP l = new KisPaintLayer(img.data(), "projection", OPACITY_OPAQUE, pd);

    vKisAnnotationSP_it beginIt = img->beginAnnotations();
    vKisAnnotationSP_it endIt = img->endAnnotations();
    if (ib.buildFile(url, l, beginIt, endIt) == KisImageBuilder_RESULT_OK) {
        return KoFilter::OK;
    }
    return KoFilter::InternalError;
}

#include <magickexport.moc>

