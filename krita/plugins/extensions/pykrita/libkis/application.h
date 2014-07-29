#ifndef LIBKIS_APPLICATION_H
#define LIBKIS_APPLICATION_H

#include <QObject>
#include <krita_export.h>
class LIBKIS_EXPORT Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = 0);

signals:

public slots:

};

#endif // LIBKIS_APPLICATION_H
