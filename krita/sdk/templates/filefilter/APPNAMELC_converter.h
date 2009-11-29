#ifndef _%{APPNAMEUC}_CONVERTER_H_
#define _%{APPNAMEUC}_CONVERTER_H_

#include <stdio.h>

#include <QObject>

#include <kio/job.h>

#include "kis_types.h"
class KisDoc2;
class KisUndoAdapter;

/**
 * Image import/export plugins can use these results to report about success or failure.
 */
enum KisImageBuilder_Result {
        KisImageBuilder_RESULT_FAILURE = -400,
        KisImageBuilder_RESULT_NOT_EXIST = -300,
        KisImageBuilder_RESULT_NOT_LOCAL = -200,
        KisImageBuilder_RESULT_BAD_FETCH = -100,
        KisImageBuilder_RESULT_INVALID_ARG = -50,
        KisImageBuilder_RESULT_OK = 0,
        KisImageBuilder_RESULT_PROGRESS = 1,
        KisImageBuilder_RESULT_EMPTY = 100,
        KisImageBuilder_RESULT_BUSY = 150,
        KisImageBuilder_RESULT_NO_URI = 200,
        KisImageBuilder_RESULT_UNSUPPORTED = 300,
        KisImageBuilder_RESULT_INTR = 400,
        KisImageBuilder_RESULT_PATH = 500,
        KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE = 600
};

class %{APPNAME}Converter : public QObject {
        Q_OBJECT
    public:
        %{APPNAME}Converter(KisDoc2 *doc, KisUndoAdapter *adapter);
        virtual ~%{APPNAME}Converter();
    public:
        KisImageBuilder_Result buildImage(const KUrl& uri);
        KisImageBuilder_Result buildFile(const KUrl& uri, KisPaintLayerSP layer);
        /**
         * Retrieve the constructed image
         */
        KisImageWSP image();
    private:
        KisImageBuilder_Result decode(const KUrl& uri);
    public slots:
        virtual void cancel();
    private:
        KisImageWSP m_image;
        KisDoc2 *m_doc;
        KisUndoAdapter *m_adapter;
        bool m_stop;
        KIO::TransferJob *m_job;
};

#endif
