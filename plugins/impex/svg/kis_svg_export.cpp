#include "kis_svg_export.h"

#include <QBuffer>
#include <cstring>

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


K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_svg_export.json", registerPlugin<KisSVGExport>();)


KisSVGExport::KisSVGExport(QObject *parent, const QVariantList &)
{
}

KisSVGExport::~KisSVGExport()
{
}

KisImportExportErrorCode KisSVGExport::convert(KisDocument *document, QIODevice *io, KisPropertiesConfigurationSP /*configuration*/)
{
    if (!io->isReadable()) {
        errFile << "Cannot read svg contents";
        return ImportExportCodes::NoAccessToRead;
    }

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
