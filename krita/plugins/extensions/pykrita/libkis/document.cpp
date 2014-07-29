#include "document.h"
#include "image.h"

#include <kis_doc2.h>

Document::Document(QObject *document, QObject *parent)
    : QObject(parent)
    , m_document(qobject_cast<KisDoc2*>(document))
{
}

Image *Document::image()
{
    return new Image(m_document->image().data(), this);
}
