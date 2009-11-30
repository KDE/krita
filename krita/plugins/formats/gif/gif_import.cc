#include "gif_import.h"

#include <kgenericfactory.h>

#include <KoFilterChain.h>

#include <kis_doc2.h>
#include <kis_image.h>

#include "gif_converter.h"

typedef KGenericFactory<gifImport> ImportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritagifimport, ImportFactory("kofficefilters"))

gifImport::gifImport(QObject *parent, const QStringList&) : KoFilter(parent)
{
}

gifImport::~gifImport()
{
}

KoFilter::ConversionStatus gifImport::convert(const QByteArray&, const QByteArray& to)
{
    dbgFile <<"Importing using GIFImport!";

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

        gifConverter ib(doc, doc -> undoAdapter());


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

#include <gif_import.moc>

