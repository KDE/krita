#include "document.h"
#include "image.h"

#include <kis_doc2.h>

Document::Document(KisDoc2 *document, QObject *parent)
    : QObject(parent)
    , m_document(document)
{
}

Image *Document::image()
{
    return new Image(m_document->image(), this);
}
