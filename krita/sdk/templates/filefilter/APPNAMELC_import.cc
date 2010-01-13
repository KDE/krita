#include "%{APPNAMELC}_import.h"

#include <kpluginfactory.h>

#include <KoFilterChain.h>

#include <kis_doc2.h>
#include <kis_image.h>

#include "%{APPNAMELC}_converter.h"

K_PLUGIN_FACTORY(ImportFactory, registerPlugin<%{APPNAME}Import>();)
K_EXPORT_PLUGIN(ImportFactory("kofficefilters"))

%{APPNAME}Import::%{APPNAME}Import(QObject *parent, const QVariantList&) : KoFilter(parent)
{
}

%{APPNAME}Import::~%{APPNAME}Import()
{
}

KoFilter::ConversionStatus %{APPNAME}Import::convert(const QByteArray&, const QByteArray& to)
{
    dbgFile <<"Importing using %{APPNAMEUC}Import!";

    if (to != "application/x-krita")
        return KoFilter::BadMimeType;

    KisDoc2 * doc = dynamic_cast<KisDoc2*>(m_chain->outputDocument());

    if (!doc)
        return KoFilter::CreationError;

    QString filename = m_chain -> inputFile();

    doc->prepareForImport();

    if (!filename.isEmpty()) {

        KUrl url;
        url.setPath(filename);

        if (url.isEmpty())
            return KoFilter::FileNotFound;

        %{APPNAME}Converter ib(doc, doc -> undoAdapter());


        switch (ib.buildImage(url)) {
            case KisImageBuilder_RESULT_UNSUPPORTED:
                return KoFilter::NotImplemented;
                break;
            case KisImageBuilder_RESULT_INVALID_ARG:
                return KoFilter::BadMimeType;
                break;
            case KisImageBuilder_RESULT_NO_URI:
            case KisImageBuilder_RESULT_NOT_LOCAL:
                return KoFilter::FileNotFound;
                break;
            case KisImageBuilder_RESULT_BAD_FETCH:
            case KisImageBuilder_RESULT_EMPTY:
                return KoFilter::ParsingError;
                break;
            case KisImageBuilder_RESULT_FAILURE:
                return KoFilter::InternalError;
                break;
            case KisImageBuilder_RESULT_OK:
                doc -> setCurrentImage( ib.image());
                return KoFilter::OK;
            default:
                break;
        }

    }
    return KoFilter::StorageCreationError;
}

#include <%{APPNAMELC}_import.moc>

