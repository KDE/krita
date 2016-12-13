/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_svg_import.h"

#include <kpluginfactory.h>
#include <QFileInfo>

#include <KisFilterChain.h>
#include <KisImportExportManager.h>

#include <KisDocument.h>
#include <kis_image.h>

#include <KisViewManager.h>

K_PLUGIN_FACTORY_WITH_JSON(SVGImportFactory, "krita_svg_import.json", registerPlugin<KisSVGImport>();)

KisSVGImport::KisSVGImport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

KisSVGImport::~KisSVGImport()
{
}

#include <SvgParser.h>
#include <KoColorSpaceRegistry.h>
#include "kis_shape_layer.h"
#include <KoShapeBasedDocumentBase.h>

KisImportExportFilter::ConversionStatus KisSVGImport::convert(const QByteArray&, const QByteArray& to, KisPropertiesConfigurationSP configuration)
{
    dbgFile << "Importing using SVGImport!";

    if (to != "application/x-krita")
        return KisImportExportFilter::BadMimeType;

    KisDocument * doc = outputDocument();

    if (!doc) {
        return KisImportExportFilter::NoDocumentCreated;
    }

    QString filename = inputFile();

    ENTER_FUNCTION() << ppVar(filename);

    doc -> prepareForImport();

    if (filename.isEmpty() || !QFileInfo(filename).exists()) {
        return KisImportExportFilter::FileNotFound;
    }

    const QString baseXmlDir = QFileInfo(filename).canonicalPath();
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    const qreal resolutionPPI = 100;
    const qreal resolution = resolutionPPI / 72.0;

    QSizeF fragmentSize;
    QList<KoShape*> shapes =
            KisShapeLayer::createShapesFromSvg(&file, baseXmlDir,
                                               QRectF(0,0,1200,800), resolutionPPI,
                                               doc->shapeController()->resourceManager(),
                                               &fragmentSize);
    ENTER_FUNCTION() << ppVar(shapes.size());


    QRectF rawImageRect(QPointF(), fragmentSize);
    QRect imageRect(rawImageRect.toAlignedRect());

    const KoColorSpace* cs = KoColorSpaceRegistry::instance()->rgb8();
    KisImageSP image = new KisImage(doc->createUndoStore(), imageRect.width(), imageRect.height(), cs, "svg image");
    image->setResolution(resolution, resolution);
    doc->setCurrentImage(image);

    KisShapeLayerSP shapeLayer =
            new KisShapeLayer(doc->shapeController(), image,
                              i18n("Vector Layer"),
                              OPACITY_OPAQUE_U8);


    //        if (!shapes.isEmpty()) {
    //            dbgKrita << "Could not load vector layer!";
    //            return KisImportExportFilter::CreationError;
    //        }

    Q_FOREACH (KoShape *shape, shapes) {
        shapeLayer->addShape(shape);
    }


    image->addNode(shapeLayer);
    return KisImportExportFilter::OK;
}

#include <kis_svg_import.moc>
