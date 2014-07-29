#ifndef LIBKIS_IMAGE_H
#define LIBKIS_IMAGE_H

#include <QObject>
#include <kis_image.h>
#include <krita_export.h>

class LIBKIS_EXPORT Image : public QObject
{
    Q_OBJECT
public:
    explicit Image(QObject *image, QObject *parent = 0);

signals:

public slots:
private:
    KisImageWSP m_image;
};

#endif // LIBKIS_IMAGE_H
