#ifndef LIBKIS_DOCUMENT_H
#define LIBKIS_DOCUMENT_H

#include <QObject>
#include <krita_export.h>

class KisDoc2;
class Image;

class LIBKIS_EXPORT Document : public QObject
{
    Q_OBJECT
public:
    explicit Document(KisDoc2 *document, QObject *parent = 0);

    Image *image();

signals:

public slots:
private:

    KisDoc2 *m_document;
};

#endif // LIBKIS_DOCUMENT_H
