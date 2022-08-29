#include "kis_svg_export.h"

#include <QBuffer>
#include <cstring>
#include <QMessageBox>

#include <KisDocument.h>
#include <KisImportExportManager.h>
#include <KisExportCheckRegistry.h>
#include <KisImportExportErrorCode.h>
#include <KoColorModelStandardIds.h>
#include <KoColorSpaceRegistry.h>
#include <kis_assert.h>
#include <kis_debug.h>
#include <kis_group_layer.h>
#include <kis_image_animation_interface.h>
#include <kis_iterator_ng.h>
#include <kis_meta_data_backend_registry.h>
#include <kis_paint_layer.h>
#include <kpluginfactory.h>
#include "kis_shape_layer.h"

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <kis_scalable_vector_graphics_save_context.h>
#include <kis_scalable_vector_graphics_save_visitor.h>

#include <SvgWriter.h>

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_svg_export.json", registerPlugin<KisSVGExport>();)


KisSVGExport::KisSVGExport(QObject *parent, const QVariantList &)
{
}

KisSVGExport::~KisSVGExport()
{
}

KisImportExportErrorCode KisSVGExport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP /*configuration*/)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(io->isWritable(), ImportExportCodes::NoAccessToWrite);
    KoStore* store = KoStore::createStore(io, KoStore::Write, "image/svg", KoStore::Auto);

    if (!store) {
        delete store;
        return ImportExportCodes::Failure;
    }

    //KisScalableVectorGraphicsSaveContext svgSaveContext(store);
    KisScalableVectorGraphicsSaveVisitor svgVisitor(io, {document->preActivatedNode()});

    //document->image()->rootLayer()->accept(svgVisitor);
    KisImageSP image = document->savingImage();
    const auto bounds = image->bounds();
    const auto *const cs = image->colorSpace();

//    const QSizeF sizeInPx = document->view->image()->bounds().size();
//    const QSizeF sizeInPt(sizeInPx.width() / document->view->image()->xRes(),
//                          sizeInPx.height() / document->view->image()->yRes());



    KisGroupLayerSP rootLayer = image->rootLayer();
    QList<KoShapeLayer*> svgLayers ;
    KisShapeLayerSP shapeLayer = qobject_cast<KisShapeLayer*>(rootLayer->firstChild().data());
    qDebug() << shapeLayer->objectName();
    QList<KoShape*> shapes = shapeLayer->shapes();
    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

/*
    while (rootLayer->nextSibling() != 0){
        qDebug() << "Going through layer " << image->objectName();

        for (int i = 0; i < rootLayer->childCount(); i++){

        }
        //KisLayerSP currentLayer = qobject_cast<KisLayer*>(rootLayer->nextSibling().data());
        KisShapeLayerSP shapeLayer = qobject_cast<KisShapeLayer*>(rootLayer->nextSibling().data());
        if (!shapeLayer) {
            return ImportExportCodes::Failure;
        }

        // so this is a group layer and we need to get into the child layers
        //KisLayerSP shapesLayer = qobject_cast<KisShapeLayer*>(rootLayer->nextSibling().data());
        QList<KoShape*> shapes = shapeLayer->shapes();
        std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
        svgLayers.append(shapes);

        rootLayer = rootLayer->nextSibling();
        }
*/

    QSizeF pageSize(qreal(10.0), qreal(10));
    //SvgWriter writer(shapes);

//    if (!writer.save(*io, pageSize)) {
//        qDebug() << "Did not save: "<< document->objectName();
//        //QMessageBox::warning(this, i18nc("@title:window", "Krita"), i18n("Could not save to svg: %1", document->objectName()));
//    }


    return ImportExportCodes::OK;
}

void KisSVGExport::initializeCapabilities()
{
    QList<QPair<KoID, KoID>> supportedColorModels;
    addCapability(KisExportCheckRegistry::instance()->get("sRGBProfileCheck")->create(KisExportCheckBase::SUPPORTED));
    addCapability(KisExportCheckRegistry::instance()->get("MultiLayerCheck")->create(KisExportCheckBase::PARTIALLY));

    supportedColorModels << QPair<KoID, KoID>() << QPair<KoID, KoID>(RGBAColorModelID, Integer8BitsColorDepthID);

    addSupportedColorModels(supportedColorModels, "SVG");

}


#include "kis_svg_export.moc"
