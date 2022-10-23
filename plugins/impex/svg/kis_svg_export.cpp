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
#include "SvgSavingContext.h"
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

    KisImageWSP image = document->image();
    const QSizeF sizeInPx = QSizeF(image->bounds().size());
    const QSizeF pageSize(sizeInPx.width() / image->xRes(),
                      sizeInPx.height() / image->yRes());

    SvgSavingContext savingContext(*io, true);

    KisScalableVectorGraphicsSaveVisitor svgVisitor(io, {document->preActivatedNode()}, pageSize, &savingContext);

    document->image()->rootLayer()->accept(svgVisitor);


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
