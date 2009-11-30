#include "gif_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kapplication.h>
#include <kdialog.h>
#include <kgenericfactory.h>

#include <KoFilterChain.h>
#include <KoColorSpaceConstants.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "gif_converter.h"

class KisExternalLayer;

typedef KGenericFactory<gifExport> ExportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritagifexport, ExportFactory("kofficefilters"))

gifExport::gifExport(QObject *parent, const QStringList&) : KoFilter(parent)
{
}

gifExport::~gifExport()
{
}

KoFilter::ConversionStatus gifExport::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile <<"GIF export! From:" << from <<", To:" << to <<"";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    KisDoc2 *output = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!output)
        return KoFilter::CreationError;


    if (filename.isEmpty()) return KoFilter::FileNotFound;

    KUrl url;
    url.setPath(filename);

    KisImageWSP img = output->image();
    Q_CHECK_PTR(img);

    gifConverter kpc(output, output->undoAdapter());

    KisPaintDeviceSP pd = new KisPaintDevice(*img->projection());
    KisPaintLayerSP l = new KisPaintLayer(img, "projection", OPACITY_OPAQUE, pd);

    KisImageBuilder_Result res;

    if ( (res = kpc.buildFile(url, l)) == KisImageBuilder_RESULT_OK) {
        dbgFile <<"success !";
        return KoFilter::OK;
    }
    dbgFile <<" Result =" << res;
    return KoFilter::InternalError;
}

#include <gif_export.moc>

