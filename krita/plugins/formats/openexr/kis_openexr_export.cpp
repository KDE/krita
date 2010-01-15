/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
#include "kis_openexr_export.h"

#include <QFile>
#include <QVector>

#include <kmessagebox.h>

#include <half.h>
#include <ImfRgbaFile.h>

#include <kpluginfactory.h>
#include <KoDocument.h>
#include <KoFilterChain.h>
#include <KoColorSpace.h>

#include "kis_doc2.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_annotation.h"
#include "kis_types.h"
#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"
#include "kis_undo_adapter.h"
#include "kis_group_layer.h"
// #include "kis_rgb_f32_colorspace.h"
// #include "kis_rgb_f16half_colorspace.h"


using namespace std;
using namespace Imf;
using namespace Imath;

K_PLUGIN_FACTORY(KisOpenEXRExportFactory, registerPlugin<KisOpenEXRExport>();)
K_EXPORT_PLUGIN(KisOpenEXRExportFactory("kofficefilters"))

KisOpenEXRExport::KisOpenEXRExport(QObject *parent, const QVariantList &) : KoFilter(parent)
{
}

KisOpenEXRExport::~KisOpenEXRExport()
{
}

KoFilter::ConversionStatus KisOpenEXRExport::convert(const QByteArray& from, const QByteArray& to)
{
    if (to != "image/x-exr" || from != "application/x-krita") {
        return KoFilter::NotImplemented;
    }

    dbgFile << "Krita exporting to OpenEXR";

    // XXX: Add dialog about flattening layers here

    KisDoc2 *doc = dynamic_cast<KisDoc2*>(m_chain -> inputDocument());
    QString filename = m_chain -> outputFile();

    if (!doc) {
        return KoFilter::CreationError;
    }

    if (filename.isEmpty()) {
        return KoFilter::FileNotFound;
    }

    KisImageWSP image = KisImageWSP(new KisImage(*doc->image()));
    Q_CHECK_PTR(image);

    // Don't store this information in the document's undo adapter
    bool undo = doc -> undoAdapter() -> undo();
    doc -> undoAdapter() -> setUndo(false);

    image -> flatten();

    KisPaintLayerSP layer = KisPaintLayerSP(dynamic_cast<KisPaintLayer*>(image->rootLayer()->firstChild().data()));
    Q_ASSERT(layer);

    doc -> undoAdapter() -> setUndo(undo);

    const KoColorSpace *cs = layer->paintDevice()->colorSpace();

    if (cs->id() != "RgbAF16") {
        // We could convert automatically, but the conversion wants to be done with
        // selectable profiles and rendering intent.
        KMessageBox::information(0, i18n("The image is using an unsupported color space. "
                                         "Please convert to 16-bit floating point RGB/Alpha "
                                         "before saving in the OpenEXR format."));

        // Don't show the couldn't save error dialog.
        doc -> setErrorMessage("USER_CANCELED");

        return KoFilter::WrongFormat;
    }

    Box2i displayWindow(V2i(0, 0), V2i(image -> width() - 1, image -> height() - 1));

    QRect dataExtent = layer -> exactBounds();
    int dataWidth = dataExtent.width();
    int dataHeight = dataExtent.height();

    Box2i dataWindow(V2i(dataExtent.left(), dataExtent.top()), V2i(dataExtent.right(), dataExtent.bottom()));

    RgbaOutputFile file(QFile::encodeName(filename), displayWindow, dataWindow, WRITE_RGBA);

    QVector<Rgba> pixels(dataWidth);

    for (int y = 0; y < dataHeight; ++y) {

        file.setFrameBuffer(pixels.data() - dataWindow.min.x - (dataWindow.min.y + y) * dataWidth, 1, dataWidth);

        KisHLineConstIterator it = layer->paintDevice()->createHLineConstIterator(dataWindow.min.x, dataWindow.min.y + y, dataWidth);
        Rgba *rgba = pixels.data();

        while (!it.isDone()) {

            // XXX: Currently we use unmultiplied data so premult it.
            half unmultipliedRed;
            half unmultipliedGreen;
            half unmultipliedBlue;
            half alpha;

            getPixel(it.rawData(), &unmultipliedRed, &unmultipliedGreen, &unmultipliedBlue, &alpha);
            rgba -> r = unmultipliedRed * alpha;
            rgba -> g = unmultipliedGreen * alpha;
            rgba -> b = unmultipliedBlue * alpha;
            rgba -> a = alpha;
            ++it;
            ++rgba;
        }
        file.writePixels();
    }

    //vKisAnnotationSP_it beginIt = image -> beginAnnotations();
    //vKisAnnotationSP_it endIt = image -> endAnnotations();
    return KoFilter::OK;
}

void KisOpenEXRExport::getPixel(const quint8 *src, half *red, half *green, half *blue, half *alpha) const
{
    struct Pixel {
        half blue;
        half green;
        half red;
        half alpha;
    };
    const Pixel *srcPixel = reinterpret_cast<const Pixel *>(src);

    *red = srcPixel->red;
    *green = srcPixel->green;
    *blue = srcPixel->blue;
    *alpha = srcPixel->alpha;
}


#include "kis_openexr_export.moc"

