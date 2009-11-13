#include "%{APPNAMELC}_converter.h"

#include <kapplication.h>

#include <kio/netaccess.h>
#include <kio/deletejob.h>

#include <kis_doc2.h>
#include <kis_image.h>
#include <kis_paint_layer.h>
#include <kis_undo_adapter.h>

%{APPNAME}Converter::%{APPNAME}Converter(KisDoc2 *doc, KisUndoAdapter *adapter)
{
    m_doc = doc;
    m_adapter = adapter;
    m_job = 0;
    m_stop = false;
}

%{APPNAME}Converter::~%{APPNAME}Converter()
{
}

KisImageBuilder_Result %{APPNAME}Converter::decode(const KUrl& uri)
{
    // open the file
#if 0
     FILE *fp = fopen(QFile::encodeName(uri.toLocalFile()), "rb");
    if (!fp)
    {
        return (KisImageBuilder_RESULT_NOT_EXIST);
    }
    // Creating the KisImageWSP
    if(!m_img) {
        m_img = new KisImage(m_doc->undoAdapter(),  cinfo.image_width,  cinfo.image_height, cs, "built image");
        Q_CHECK_PTR(m_img);
        m_img->lock();
    }
    KisPaintLayerSP layer = new KisPaintLayer(m_img.data(), m_img->nextLayerName(), quint8_MAX));
#endif

    return KisImageBuilder_RESULT_OK;
}



KisImageBuilder_Result %{APPNAME}Converter::buildImage(const KUrl& uri)
{
    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!KIO::NetAccess::exists(uri, false, QApplication::activeWindow())) {
        return KisImageBuilder_RESULT_NOT_EXIST;
    }

    // We're not set up to handle asynchronous loading at the moment.
    KisImageBuilder_Result result = KisImageBuilder_RESULT_FAILURE;
    QString tmpFile;

    if (KIO::NetAccess::download(uri, tmpFile, qApp->activeWindow())) {
        KUrl uriTF;
        uriTF.setPath( tmpFile );
        result = decode(uriTF);
        KIO::NetAccess::removeTempFile(tmpFile);
    }

    return result;
}


KisImageWSP %{APPNAME}Converter::image()
{
    return m_img;
}


KisImageBuilder_Result %{APPNAME}Converter::buildFile(const KUrl& uri, KisPaintLayerSP layer)
{
    if (!layer)
        return KisImageBuilder_RESULT_INVALID_ARG;

    KisImageWSP img = layer->image();
    if (!img)
        return KisImageBuilder_RESULT_EMPTY;

    if (uri.isEmpty())
        return KisImageBuilder_RESULT_NO_URI;

    if (!uri.isLocalFile())
        return KisImageBuilder_RESULT_NOT_LOCAL;
    // Open file for writing
#if 0
    FILE *fp = fopen(QFile::encodeName(uri.path()), "wb");
    if (!fp)
    {
        return (KisImageBuilder_RESULT_FAILURE);
    }
    uint height = img->height();
    uint width = img->width();
#endif

    return KisImageBuilder_RESULT_OK;
}


void %{APPNAME}Converter::cancel()
{
    m_stop = true;
}

#include "%{APPNAMELC}_converter.moc"

