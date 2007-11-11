#include "KoOdf.h"

KoOdf::DocumentData KoOdf::sm_documentData[] = {
    { "application/vnd.oasis.opendocument.text", "application/vnd.oasis.opendocument.text-template", "office:text" },
    { "application/vnd.oasis.opendocument.graphics", "application/vnd.oasis.opendocument.graphics-template", "office:drawing" },
    { "application/vnd.oasis.opendocument.presentation", "application/vnd.oasis.opendocument.presentation-template", "office:presentation" },
    { "application/vnd.oasis.opendocument.spreadsheet", "application/vnd.oasis.opendocument.spreadsheet-template", "office:spreadsheet" },
    { "application/vnd.oasis.opendocument.chart", "application/vnd.oasis.opendocument.chart-template", "office:chart" },
    { "application/vnd.oasis.opendocument.image", "application/vnd.oasis.opendocument.image-template", "office:image" },
    // TODO what is the element for a formula check if bodyContentElement is ok
    { "application/vnd.oasis.opendocument.formula", "application/vnd.oasis.opendocument.formula-template", "office:XXX" }
};
//"application/vnd.oasis.opendocument.text-master"
//"application/vnd.oasis.opendocument.text-web"

const char * KoOdf::mimeType( DocumentType documentType )
{
    return sm_documentData[documentType].mimeType;
}

const char * KoOdf::templateMimeType( DocumentType documentType )
{
    return sm_documentData[documentType].templateMimeType;
}

const char * KoOdf::bodyContentElement( DocumentType documentType, bool withNamespace )
{
    return withNamespace ? sm_documentData[documentType].bodyContentElement : sm_documentData[documentType].bodyContentElement + 7;
}
