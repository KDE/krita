#include "%{APPNAMELC}_export.h"

#include <QCheckBox>
#include <QSlider>

#include <kapplication.h>
#include <kdialog.h>
#include <kpluginfactory.h>

#include <KoFilterChain.h>
#include <KoColorSpaceConstants.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "%{APPNAMELC}_converter.h"

class KisExternalLayer;

K_PLUGIN_FACTORY(ExportFactory, registerPlugin<%{APPNAME}Export>();)
K_EXPORT_PLUGIN(ExportFactory("kofficefilters"))

%{APPNAME}Export::%{APPNAME}Export(QObject *parent, const QVariantList&) : KoFilter(parent)
{
}

%{APPNAME}Export::~%{APPNAME}Export()
{
}

KoFilter::ConversionStatus %{APPNAME}Export::convert(const QByteArray& from, const QByteArray& to)
{
    dbgFile <<"%{APPNAMEUC} export! From:" << from <<", To:" << to <<"";

    if (from != "application/x-krita")
        return KoFilter::NotImplemented;

    KisDoc2 *output = dynamic_cast<KisDoc2*>(m_chain->inputDocument());
    QString filename = m_chain->outputFile();

    if (!output)
        return KoFilter::CreationError;


    if (filename.isEmpty()) return KoFilter::FileNotFound;

    KUrl url;
    url.setPath(filename);

    KisImageWSP image = output->image();
    Q_CHECK_PTR(image);

    %{APPNAME}Converter kpc(output, output->undoAdapter());

    KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());
    KisPaintLayerSP l = new KisPaintLayer(image, "projection", OPACITY_OPAQUE, pd);

    KisImageBuilder_Result res;

    if ( (res = kpc.buildFile(url, l)) == KisImageBuilder_RESULT_OK) {
        dbgFile <<"success !";
        return KoFilter::OK;
    }
    dbgFile <<" Result =" << res;
    return KoFilter::InternalError;
}

#include <%{APPNAMELC}_export.moc>

