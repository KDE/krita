#ifndef KOODF_H
#define KOODF_H

#include <koodf_export.h>

class KOODF_EXPORT KoOdf
{
public:
    enum DocumentType {
        Text,
        Graphics,
        Presentation,
        Spreadsheet,
        Chart,
        Image,
        Formula
    };

    static const char * mimeType( DocumentType documentType );
    static const char * templateMimeType( DocumentType documentType );
    static const char * bodyContentElement( DocumentType documentType, bool withNamespace );

private:
    struct DocumentData {
        const char * mimeType;
        const char * templateMimeType;
        const char * bodyContentElement;
    };

    static DocumentData sm_documentData[];
};

#endif /* KOODF_H */
